#include "event_generator.h"

EventGenerator::EventGenerator( const Event::User min_user,
                                const Event::User max_user,
                                const EventGenerator::enum_type lowest_event,
                                const EventGenerator::enum_type highest_event,
                                const uint8_t min_count_letters,
                                const uint8_t max_count_letters,
                                const char min_letter,
                                const char max_letter,
                                EventGenerator::time_type min_time,
                                EventGenerator::time_type max_time,
                                const int32_t min_amount,
                                const int32_t max_amount )
    : _generator( _random_device() )
    , _user_distribution( min_user, max_user )
    , _event_distribution( lowest_event, highest_event )
    , _name_length_distribution( min_count_letters, max_count_letters )
    , _letter_distribution( min_letter, max_letter )
    , _time_distribution( min_time, max_time )
    , _amount_distribution( min_amount, max_amount )
{
}

std::unique_ptr< Event > EventGenerator::generateEvent(const std::chrono::nanoseconds time)
{
	BaseEvent base_event( get_random_user(), get_random_event() );
	while ( !is_allowed_event( base_event ) ) {
		base_event.setType( get_random_event() );
	}

	std::unique_ptr< Event > event;
	switch ( base_event.type() ) {
		case Event::Type::user_registered: {
			auto name = get_random_name();
			event.reset( new UserRegisteredEvent( base_event.user(), name ) );
			_registered_users.emplace( base_event.user(), name );
			break;
		}
		case Event::Type::user_renamed: {
			auto name = get_random_name();
			event.reset( new UserRenamedEvent( base_event.user(), name ) );
			_registered_users.emplace( base_event.user(), name );
			break;
		}
		case Event::Type::user_deal_won: {
			auto random_time = get_random_time();
			auto amount = get_random_amount();
			event.reset( new UserDealWonEvent( base_event.user(), time + random_time, amount ) );
			break;
		}
		case Event::Type::user_connected: {
			event.reset( new UserConnectedEvent( base_event.user() ) );
			_connected_users.emplace( base_event.user() );
			break;
		}
		case Event::Type::user_disconnected: {
			event.reset( new UserDisconnectedEvent( base_event.user() ) );
			_connected_users.erase( base_event.user() );
			break;
		}
		case Event::Type::undefined: {
			break;
		}
	}

	return event;
}

Event::User EventGenerator::get_random_user()
{
	return _user_distribution( _generator );
}

Event::Type EventGenerator::get_random_event()
{
	return static_cast< Event::Type >( _event_distribution( _generator ) );
}

std::string EventGenerator::get_random_name()
{
	std::string name( _name_length_distribution( _generator ), '\0' );
	for ( auto& symbol : name ) {
		symbol = _letter_distribution( _generator );
	}

	return name;
}

std::chrono::nanoseconds EventGenerator::get_random_time()
{
	return std::chrono::nanoseconds( _time_distribution( _generator ) );
}

int32_t EventGenerator::get_random_amount()
{
	return _amount_distribution( _generator );
}

bool EventGenerator::is_allowed_event( const Event& event )
{
	if ( _registered_users.find( event.user() ) == _registered_users.cend() ) {
		return Event::Type::user_registered == event.type();
	}

	if ( _connected_users.find( event.user() ) == _connected_users.cend() ) {
		return Event::Type::user_connected == event.type();
	}

	return Event::Type::user_renamed == event.type() || Event::Type::user_deal_won == event.type()
	       || Event::Type::user_disconnected == event.type();
}
