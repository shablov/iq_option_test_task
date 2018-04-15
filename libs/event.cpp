#include "event.h"

#include <iostream>

Event::Event( User user, Event::Type type ) : _user( user ), _type( type )
{
}

Event::Type Event::type() const
{
	return _type;
}

void Event::setType( const Type& type )
{
	_type = type;
}

const char* Event::type_name() const
{
	switch ( _type ) {
		case Type::undefined:
			return "undefined";
		case Type::user_registered:
			return "user_registered";
		case Type::user_renamed:
			return "user_renamed";
		case Type::user_deal_won:
			return "user_deal_won";
		case Type::user_connected:
			return "user_connected";
		case Type::user_disconnected:
			return "user_disconnected";
	}
}

Event::User Event::user() const
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
	//	out_stream << event.type_name() << " " << event._user;
	out_stream << static_cast< std::underlying_type_t< Event::Type > >( event._type ) << " " << event._user;
	return event.to_stream( out_stream );
}

std::istream& operator>>( std::istream& in_stream, Event& event )
{
	if ( Event::Type::undefined == event._type ) {
		return in_stream >> *reinterpret_cast< std::underlying_type_t< Event::Type >* >( &event._type ) >> event._user;
	}
	else {
		return event.finish_reading( in_stream );
	}
}

UserRegisteredEvent::UserRegisteredEvent( const Event& event ) : Event( event )
{
}

UserRegisteredEvent::UserRegisteredEvent( User user, std::string_view name )
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

void UserRegisteredEvent::setName( const std::string& name )
{
	_name = name;
}

UserRenamedEvent::UserRenamedEvent( const Event& event ) : Event( event )
{
}

UserRenamedEvent::UserRenamedEvent( User user, std::string_view name )
    : Event( user, Type::user_renamed ), _name( name )
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

void UserRenamedEvent::setName( const std::string& name )
{
	_name = name;
}

UserDealWonEvent::UserDealWonEvent( const Event& event ) : Event( event )
{
}

UserDealWonEvent::UserDealWonEvent( User user, std::chrono::nanoseconds time, int64_t amount )
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

int64_t UserDealWonEvent::amount() const
{
	return _amount;
}

void UserDealWonEvent::setAmount( const int64_t& amount )
{
	_amount = amount;
}

std::chrono::nanoseconds UserDealWonEvent::time() const
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

UserConnectedEvent::UserConnectedEvent( User user ) : Event( user, Type::user_connected )
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

UserDisconnectedEvent::UserDisconnectedEvent( User user ) : Event( user, Type::user_disconnected )
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
