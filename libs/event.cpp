#include "event.h"

#include <iostream>

std::ostream& operator<<( std::ostream& out_stream, const Event::Type& type )
{
	out_stream << static_cast< std::underlying_type_t< Event::Type > >( type );
	return out_stream;
}

std::istream& operator>>( std::istream& in_stream, Event::Type& type )
{
	type = Event::Type::undefined;

	std::underlying_type_t< Event::Type > tmp_type;
	in_stream >> tmp_type;

	switch ( static_cast< Event::Type >( tmp_type ) ) {
		case Event::Type::user_registered:
		case Event::Type::user_renamed:
		case Event::Type::user_deal_won:
		case Event::Type::user_connected:
		case Event::Type::user_disconnected:
		case Event::Type::undefined:
			type = static_cast< Event::Type >( tmp_type );
	}

	return in_stream;
}

Event::Event( const User user, const Type type ) : _user( user ), _type( type )
{
}

Event::Type Event::type() const noexcept
{
	return _type;
}

void Event::setType( const Type& type ) noexcept
{
	_type = type;
}

Event::User Event::user() const noexcept
{
	return _user;
}

void Event::setUser( const User& user )
{
	_user = user;
}

std::unique_ptr< Event > Event::createEvent( const Event& event )
{
	switch ( event._type ) {
		case Type::undefined:
			return nullptr;
		case Type::user_registered:
			return std::make_unique< UserRegisteredEvent >( event );
		case Type::user_renamed:
			return std::make_unique< UserRenamedEvent >( event );
		case Type::user_deal_won:
			return std::make_unique< UserDealWonEvent >( event );
		case Type::user_connected:
			return std::make_unique< UserConnectedEvent >( event );
		case Type::user_disconnected:
			return std::make_unique< UserDisconnectedEvent >( event );
	}
	return nullptr;
}

BaseEvent::BaseEvent( User user, Type type ) : Event( user, type )
{
}

std::ostream& BaseEvent::to_stream( std::ostream& out_stream ) const
{
	return out_stream;
}

std::istream& BaseEvent::finish_reading( std::istream& in_stream )
{
	return in_stream;
}

std::ostream& operator<<( std::ostream& out_stream, const Event& event )
{
	out_stream << event._type << " " << event._user;
	return event.to_stream( out_stream );
}

std::istream& operator>>( std::istream& in_stream, Event& event )
{
	if ( Event::Type::undefined == event._type ) {
		return in_stream >> event._type >> event._user;
	}
	else {
		return event.finish_reading( in_stream );
	}
}

UserRegisteredEvent::UserRegisteredEvent( const Event& event ) : Event( event )
{
}

UserRegisteredEvent::UserRegisteredEvent( const User user, const std::string_view name )
    : Event( user, Type::user_registered ), _name( name )
{
}

std::ostream& UserRegisteredEvent::to_stream( std::ostream& out_stream ) const
{
	out_stream << " " << _name;
	return out_stream;
}

std::istream& UserRegisteredEvent::finish_reading( std::istream& in_stream )
{
	in_stream.get();
	in_stream >> _name;
	return in_stream;
}

std::string UserRegisteredEvent::name() const
{
	return _name;
}

void UserRegisteredEvent::setName( const std::string_view name )
{
	_name = name;
}

UserRenamedEvent::UserRenamedEvent( const Event& event ) : Event( event )
{
}

UserRenamedEvent::UserRenamedEvent( const User user, const std::string_view name ) : Event( user, Type::user_renamed ), _name( name )
{
}

std::ostream& UserRenamedEvent::to_stream( std::ostream& out_stream ) const
{
	out_stream << " " << _name;
	return out_stream;
}

std::istream& UserRenamedEvent::finish_reading( std::istream& in_stream )
{
	in_stream.get();
	in_stream >> _name;
	return in_stream;
}

std::string UserRenamedEvent::name() const
{
	return _name;
}

void UserRenamedEvent::setName( const std::string_view name )
{
	_name = name;
}

UserDealWonEvent::UserDealWonEvent( const Event& event ) : Event( event )
{
}

UserDealWonEvent::UserDealWonEvent( const User user, const std::chrono::nanoseconds time, const int64_t amount )
    : Event( user, Type::user_deal_won ), _time( time ), _amount( amount )
{
}

std::ostream& UserDealWonEvent::to_stream( std::ostream& out_stream ) const
{
	out_stream << " " << _time.count() << " " << _amount;
	return out_stream;
}

std::istream& UserDealWonEvent::finish_reading( std::istream& in_stream )
{
	decltype( _time.count() ) tmp;
	in_stream >> tmp >> _amount;
	_time = decltype( _time )( tmp );
	return in_stream;
}

int64_t UserDealWonEvent::amount() const noexcept
{
	return _amount;
}

void UserDealWonEvent::setAmount( const int64_t& amount )
{
	_amount = amount;
}

std::chrono::nanoseconds UserDealWonEvent::time() const noexcept
{
	return _time;
}

void UserDealWonEvent::setTime( const std::chrono::nanoseconds& time )
{
	_time = time;
}

UserConnectedEvent::UserConnectedEvent( const Event& event ) : Event( event )
{
}

UserConnectedEvent::UserConnectedEvent( const User user ) : Event( user, Type::user_connected )
{
}

std::ostream& UserConnectedEvent::to_stream( std::ostream& out_stream ) const
{
	return out_stream;
}

std::istream& UserConnectedEvent::finish_reading( std::istream& in_stream )
{
	return in_stream;
}

UserDisconnectedEvent::UserDisconnectedEvent( const Event& event ) : Event( event )
{
}

UserDisconnectedEvent::UserDisconnectedEvent( const User user ) : Event( user, Type::user_disconnected )
{
}

std::ostream& UserDisconnectedEvent::to_stream( std::ostream& out_stream ) const
{
	return out_stream;
}

std::istream& UserDisconnectedEvent::finish_reading( std::istream& in_stream )
{
	return in_stream;
}
