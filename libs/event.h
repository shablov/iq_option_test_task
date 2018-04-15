#pragma once

#include <chrono>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

class Event
{
public:
	using User = int32_t;

	enum class Type : int32_t
	{
		undefined = -1,
		user_registered,
		user_renamed,
		user_deal_won,
		user_connected,
		user_disconnected,
	};

	Event( User user, Type type );
	Event() = default;
	Event( const Event& ) = default;
	Event& operator=( const Event& ) = default;
	Event( Event&& ) = default;
	Event& operator=( Event&& ) = default;
	virtual ~Event() = default;

	Type type() const;
	void setType( const Type& type );

	const char* type_name() const;

	User user() const;
	void setUser( const User& user );

	static std::unique_ptr< Event > createEvent( const Event& event );

	friend std::ostream& operator<<( std::ostream& out_stream, const Event& event );
	friend std::istream& operator>>( std::istream& in_stream, Event& event );

protected:
	User _user = -1;
	Type _type = Type::undefined;

private:
	virtual std::ostream& to_stream( std::ostream& out_stream ) const = 0;
	virtual std::istream& finish_reading( std::istream& in_stream ) = 0;
};

class BaseEvent : public Event
{
public:
	BaseEvent() = default;
	BaseEvent( User user, Type type );

private:
	std::ostream& to_stream( std::ostream& out_stream ) const override;
	std::istream& finish_reading( std::istream& in_stream ) override;
};

class UserRegisteredEvent : public Event
{
public:
	UserRegisteredEvent( const Event& event );
	UserRegisteredEvent( User user, std::string_view name );

	std::string name() const;
	void setName( const std::string& name );

private:
	std::ostream& to_stream( std::ostream& out_stream ) const override;
	std::istream& finish_reading( std::istream& in_stream ) override;

	std::string _name;
};

class UserRenamedEvent : public Event
{
public:
	UserRenamedEvent( const Event& event );
	UserRenamedEvent( User user, std::string_view name );

	std::string name() const;
	void setName( const std::string& name );

private:
	std::ostream& to_stream( std::ostream& out_stream ) const override;
	std::istream& finish_reading( std::istream& in_stream ) override;

	std::string _name;
};

class UserDealWonEvent : public Event
{
public:
	UserDealWonEvent( const Event& event );
	UserDealWonEvent( User user, std::chrono::nanoseconds time, int64_t amount );

	std::chrono::nanoseconds time() const;
	void setTime( const std::chrono::nanoseconds& time );

	int64_t amount() const;
	void setAmount( const int64_t& amount );

private:
	std::ostream& to_stream( std::ostream& out_stream ) const override;
	std::istream& finish_reading( std::istream& in_stream ) override;

	std::chrono::nanoseconds _time;
	int64_t _amount;
};

class UserConnectedEvent : public Event
{
public:
	UserConnectedEvent( const Event& event );
	UserConnectedEvent( User user );

private:
	std::ostream& to_stream( std::ostream& out_stream ) const override;
	std::istream& finish_reading( std::istream& in_stream ) override;
};

class UserDisconnectedEvent : public Event
{
public:
	UserDisconnectedEvent( const Event& event );
	UserDisconnectedEvent( User user );

private:
	std::ostream& to_stream( std::ostream& out_stream ) const override;
	std::istream& finish_reading( std::istream& in_stream ) override;
};
