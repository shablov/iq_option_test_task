#include "events_handler.h"

#include <algorithm>

EventsHandler::EventsHandler( PacketsHandler& packetsHandler ) : _packets_handler( packetsHandler )
{
}

void EventsHandler::put( std::unique_ptr< Event >&& event )
{
	std::unique_lock lock( _mutex );
	_unhandled_events.push_back( std::move( event ) );
	_condition_variable.notify_one();
}

void EventsHandler::procesing()
{
	_stopped.store( false );
	while ( pop() ) {
		for ( const auto& event : _processing_events ) {
			switch ( event->type() ) {
				case Event::Type::undefined: {
					break;
				}
				case Event::Type::user_registered: {
					registered( static_cast< UserRegisteredEvent& >( *event ) );
					break;
				}
				case Event::Type::user_renamed: {
					renamed( static_cast< UserRenamedEvent& >( *event ) );
					break;
				}
				case Event::Type::user_deal_won: {
					dealWon( static_cast< UserDealWonEvent& >( *event ) );
					break;
				}
				case Event::Type::user_connected: {
					connected( static_cast< UserConnectedEvent& >( *event ) );
					break;
				}
				case Event::Type::user_disconnected: {
					disconnected( static_cast< UserDisconnectedEvent& >( *event ) );
					break;
				}
			}
		}
	}
}

void EventsHandler::stopProcessing()
{
	_stopped.store( true );
	_condition_variable.notify_one();
}

bool EventsHandler::pop()
{
	_processing_events.clear();
	std::unique_lock lock( _mutex );
	_condition_variable.wait( lock, [this] { return _stopped.load() || !_unhandled_events.empty(); } );

	if ( !_stopped.load() ) {
		_processing_events.push_back( std::move( _unhandled_events.front() ) );
		_unhandled_events.pop_front();
	}

	return !_stopped.load();
}

void EventsHandler::registered( const UserRegisteredEvent& event )
{
	addNewUser( event.user(), event.name() );
}

void EventsHandler::connected( const UserConnectedEvent& event )
{
	_connected_users.emplace( event.user() );

	sendUserStatistics( event.user() );
}

void EventsHandler::renamed( const UserRenamedEvent& event )
{
	_registered_users[ event.user() ] = event.name();
}

void EventsHandler::dealWon( const UserDealWonEvent& event )
{
	updateUserStatistics( event.user(), event.amount(), event.time() );
}

void EventsHandler::disconnected( const UserDisconnectedEvent& event )
{
	_connected_users.erase( event.user() );
}

void EventsHandler::addNewUser( const Event::User user, const std::string_view name )
{
	_registered_users.emplace( user, name );
	_statistics.emplace( user, 0 );
	_sorted_statistics.emplace( 0, user );
}

Packet EventsHandler::userStatistic( const Event::User user ) const
{
	const auto self = userRank( user );
	const auto position = static_cast< size_t >( std::distance( _sorted_statistics.cbegin(), self ) ) + 1;

	return {user, position, topStatistic(), neigborsStatistic( self, position )};
}

SortedStatistic::const_iterator EventsHandler::userRank( const Event::User user ) const
{
	if ( auto it = _statistics.find( user ); it != _statistics.cend() ) {
		const auto& [ id, amount ] = *it;
		return _sorted_statistics.find( {amount, user} );
	}
	return _sorted_statistics.cend();
}

SortedStatistic EventsHandler::topStatistic() const
{
	const auto& statistics = _sorted_statistics;
	return SortedStatistic( statistics.cbegin(),
	                        statistics.size() > neighbors_count ? std::next( statistics.cbegin(), neighbors_count ) : statistics.cend() );
}

SortedStatistic EventsHandler::neigborsStatistic( const SortedStatistic::const_iterator rank, const size_t position ) const
{
	const auto& statistics = _sorted_statistics;
	return SortedStatistic( position > neighbors_count ? std::next( rank, -neighbors_count ) : statistics.cbegin(),
	                        statistics.size() - position > neighbors_count ? std::next( rank, neighbors_count + 1 ) : statistics.cend() );
}

void EventsHandler::sendUserStatistics( const Event::User user )
{
	sendPacket( userStatistic( user ) );
}

void EventsHandler::sendPacket( Packet&& packet )
{
	_packets_handler.put( std::move( packet ) );
}

void EventsHandler::addUserAmount( const Event::User user, const int64_t amount )
{
	const auto lastAmount = _statistics[ user ];
	const auto total = lastAmount + amount;
	_statistics[ user ] = total;
	if ( auto node = _sorted_statistics.extract( {lastAmount, user} ); !node.empty() ) {
		node.value().first = total;
		_sorted_statistics.insert( std::move( node ) );
	}
	else {
		_sorted_statistics.emplace( total, user );
	}
}

void EventsHandler::updateUserStatistics( const Event::User user, const int64_t amount, const std::chrono::nanoseconds time )
{
	using namespace std::chrono_literals;
	static const constexpr auto week = 7 * 24h;
	const auto week_time = time % week;

	if ( isNewWeek( week_time ) ) {
		clearStatistics();
	}

	addUserAmount( user, amount );

	if ( isNextMinute( week_time ) ) {
		sendPackets();
	}

	updateTime( week_time );
}

bool EventsHandler::isNewWeek( const std::chrono::nanoseconds time ) const noexcept
{
	return last_update_week_time > time;
}

void EventsHandler::clearStatistics()
{
	_sorted_statistics.clear();
	for ( auto& [ user, amount ] : _statistics ) {
		amount = 0;
		_sorted_statistics.emplace( 0, user );
	}
}

bool EventsHandler::isNextMinute( const std::chrono::nanoseconds time ) const noexcept
{
	using namespace std::chrono_literals;
	return last_update_week_time / 1min != time / 1min;
}

void EventsHandler::sendPackets()
{
	std::vector< Packet > packets;

	const auto& users = _connected_users;
	packets.reserve( users.size() );

	const auto transform_operation = [this]( const auto& v ) { return userStatistic( v ); };
	std::transform( users.cbegin(), users.cend(), std::back_insert_iterator( packets ), transform_operation );

	_packets_handler.put( std::move( packets ) );
}

void EventsHandler::updateTime( const std::chrono::nanoseconds time ) noexcept
{
	last_update_week_time = time;
}
