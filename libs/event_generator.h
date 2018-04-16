#pragma

#include <chrono>
#include <cstddef>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "event.h"

class EventGenerator
{
	using enum_type = std::underlying_type_t< Event::Type >;
	using time_type = decltype( std::chrono::nanoseconds::period::den );

public:
	EventGenerator( const Event::User min_user = _min_user,
	                const Event::User max_user = _max_user,
	                const enum_type lowest_event = _lowest_event,
	                const enum_type highest_event = _highest_event,
	                const uint8_t min_count_letters = _min_count_letters,
	                const uint8_t max_count_letters = _max_count_letters,
	                const char min_letter = _min_letter,
	                const char max_letter = _max_letter,
	                time_type min_time = std::chrono::nanoseconds::zero().count(),
	                time_type max_time = std::chrono::nanoseconds::period::den,
	                const int32_t min_amount = _min_amount,
	                const int32_t max_amount = _max_amount );

	std::unique_ptr< Event > generateEvent(const std::chrono::nanoseconds time);

private:
	Event::User get_random_user();

	Event::Type get_random_event();

	std::string get_random_name();

	std::chrono::nanoseconds get_random_time();

	int32_t get_random_amount();

	bool is_allowed_event( const Event& event );

	std::unordered_map< Event::User, std::string > _registered_users;
	std::unordered_set< Event::User > _connected_users;

	std::random_device _random_device;
	std::mt19937 _generator;

	static constexpr inline Event::User _min_user = 1;
	static constexpr inline Event::User _max_user = 1000;
	std::uniform_int_distribution< Event::User > _user_distribution;

	static constexpr inline enum_type _lowest_event = static_cast< enum_type >( Event::Type::user_registered );
	static constexpr inline enum_type _highest_event = static_cast< enum_type >( Event::Type::user_disconnected );
	std::uniform_int_distribution< enum_type > _event_distribution;

	static constexpr inline uint8_t _min_count_letters = 6;
	static constexpr inline uint8_t _max_count_letters = 12;
	std::uniform_int_distribution< uint8_t > _name_length_distribution;

	static constexpr inline char _min_letter = 'a';
	static constexpr inline char _max_letter = 'z';
	std::uniform_int_distribution< char > _letter_distribution;

	std::uniform_int_distribution< time_type > _time_distribution;

	static constexpr inline int32_t _min_amount = -1000;
	static constexpr inline int32_t _max_amount = +1000;
	std::uniform_int_distribution< int32_t > _amount_distribution;
};
