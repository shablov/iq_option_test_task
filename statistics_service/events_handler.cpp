#include "events_handler.h"

EventsHandler::EventsHandler( PacketsHandler& packetsHandler ) : _packets_handler( packetsHandler )
{
}

void EventsHandler::put( std::unique_ptr< Event >&& event )
{
	std::unique_lock lock( _mutex );
	_unhandled_events.push_back( std::move( event ) );
	_condition_variable.notify_one();
}

void EventsHandler::proccesing()
{
	_stopped.store( false );
	while ( pop() ) {
		for ( auto& event : _processing_events ) {
			switch ( event->type() ) {
				case Event::Type::undefined: {
					break;
				}
				case Event::Type::user_registered: {
					registered( *static_cast< UserRegisteredEvent* >( event.get() ) );
					break;
				}
				case Event::Type::user_renamed: {
					renamed( *static_cast< UserRenamedEvent* >( event.get() ) );
					break;
				}
				case Event::Type::user_deal_won: {
					dealWon( *static_cast< UserDealWonEvent* >( event.get() ) );
					break;
				}
				case Event::Type::user_connected: {
					connected( *static_cast< UserConnectedEvent* >( event.get() ) );
					break;
				}
				case Event::Type::user_disconnected: {
					disconnected( *static_cast< UserDisconnectedEvent* >( event.get() ) );
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
	_condition_variable.wait( lock, [this]() { return !( !_stopped.load() && _unhandled_events.empty() ); } );

	//	_processing_events.assign( std::move_iterator( _unhandled_events.begin() ),
	//	                           std::move_iterator( _unhandled_events.end() ) );
	//	_unhandled_events.clear();
	_processing_events.push_back( std::move( _unhandled_events.front() ) );
	_unhandled_events.pop_front();

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

void EventsHandler::addNewUser( Event::User user, std::string_view name )
{
	_registered_users.emplace( user, name );
	_statistics.emplace( user, 0 );
	_sorted_statistics.emplace( 0, user );
}

auto EventsHandler::getUserRank( Event::User user )
{
	return _sorted_statistics.find( std::make_pair( _statistics[ user ], user ) );
}

Packet EventsHandler::getUserStatistics( Event::User user )
{
	static constexpr int max_count = 10;
	auto total_size = _sorted_statistics.size();

	auto self = getUserRank( user );

	auto begin_top = _sorted_statistics.cbegin();
	auto end_top = ( total_size > max_count ) ? std::next( begin_top, max_count ) : _sorted_statistics.cend();

	auto position = std::distance( begin_top, self ) + 1;
	auto begin_near = position > max_count ? std::next( self, -max_count ) : begin_top;
	auto end_near = ( total_size - static_cast< size_t >( position ) > max_count ) ? std::next( self, max_count + 1 ) :
	                                                                                 _sorted_statistics.cend();

	return {user, position, SortedStatistic( begin_top, end_top ), SortedStatistic( begin_near, end_near )};
}

void EventsHandler::sendUserStatistics( Event::User user )
{
	sendPacket( getUserStatistics( user ) );
}

void EventsHandler::sendPacket( Packet&& packet )
{
	_packets_handler.put( std::move( packet ) );
}

void EventsHandler::updateUserStatistics( Event::User user, int64_t amount, std::chrono::nanoseconds time )
{
	using namespace std::chrono_literals;
	static constexpr auto week = 7 * 24h;
	time = time % week;

	if ( isNewWeek( time ) ) {
		clearStatistics();
	}

	auto lastAmount = _statistics[ user ];
	auto node = _sorted_statistics.extract( std::make_pair( lastAmount, user ) );

	auto total = lastAmount + amount;
	_statistics[ user ] = total;
	if ( !node.empty() ) {
		node.value().first = total;
		_sorted_statistics.insert( std::move( node ) );
	}
	else {
		_sorted_statistics.emplace( total, user );
	}

	if ( isNextMinute( time ) ) {
		sendPackets();
	}
	updateTime( time );
}

bool EventsHandler::isNextMinute( std::chrono::nanoseconds time ) const
{
	using namespace std::chrono_literals;
	return last_update_time / 1min != time / 1min;
}

bool EventsHandler::isNewWeek( std::chrono::nanoseconds time ) const
{
	return last_update_time > time;
}

void EventsHandler::clearStatistics()
{
	std::cout << "clearStatistics()" << std::endl;
	printAll();
	_sorted_statistics.clear();
	for ( auto& e : _statistics ) {
		e.second = 0;
		_sorted_statistics.emplace( e.second, e.first );
	}
}

void EventsHandler::sendPackets()
{
	std::vector< Packet > packets;
	packets.reserve( _connected_users.size() );
	for ( auto user : _connected_users ) {
		packets.push_back( getUserStatistics( user ) );
	}

	_packets_handler.replace( std::move( packets ) );
}

void EventsHandler::updateTime( std::chrono::nanoseconds time )
{
	last_update_time = time;
}
