#include <chrono>
#include <cstddef>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>

using User = uint64_t;

std::chrono::nanoseconds currentTime = std::chrono::nanoseconds::zero();

std::unordered_map< User, std::string > registered_users;
std::unordered_set< User > connected_users;

enum class Event
{
	user_registered,
	user_renamed,
	user_deal_won,
	user_connected,
	user_disconnected,
};

constexpr auto event_name( Event event )
{
	switch ( event ) {
		case Event::user_registered:
			return "user_registered";
		case Event::user_renamed:
			return "user_renamed";
		case Event::user_deal_won:
			return "user_deal_won";
		case Event::user_connected:
			return "user_connected";
		case Event::user_disconnected:
			return "user_disconnected";
	}
}

std::ostream& operator<<( std::ostream& out, Event event )
{
	return out << event_name( event );
}

template < typename Generator >
auto get_random_user( Generator& generator )
{
	static constexpr User min_user_id = std::numeric_limits< User >::min();
	static constexpr User max_user_id = 1000;

	static std::uniform_int_distribution distribution( min_user_id, max_user_id );

	return distribution( generator );
}

template < typename Generator >
auto get_random_event( Generator& generator )
{
	static constexpr uint8_t events_count = static_cast< uint8_t >( Event::user_disconnected ) + 1;

	static std::uniform_int_distribution distribution( uint8_t( 0 ), events_count );

	return static_cast< Event >( distribution( generator ) );
}

template < typename Generator >
auto get_random_name( Generator& generator )
{
	static constexpr uint8_t min_count_letters = 6;
	static constexpr uint8_t max_count_letters = 12;

	static std::uniform_int_distribution name_length_distribution( min_count_letters, max_count_letters );
	static std::uniform_int_distribution letter_distribution( 'a', 'z' );

	std::string name( name_length_distribution( generator ), '\0' );
	for ( auto& symbol : name ) {
		symbol = letter_distribution( generator );
	}

	return name;
}

template < typename Generator >
auto get_random_time( Generator& generator )
{
	static auto nanoseconds = std::chrono::nanoseconds::zero();

	static std::uniform_int_distribution distribution( std::chrono::nanoseconds::zero().count(),
	                                                   std::chrono::nanoseconds::period::den );
	nanoseconds += std::chrono::nanoseconds( distribution( generator ) );

	return nanoseconds;
}

template < typename Generator >
auto get_random_amount( Generator& generator )
{
	static constexpr double min_amount = -1000;
	static constexpr double max_amount = +1000;

	static std::uniform_real_distribution distribution( min_amount, max_amount );

	return distribution( generator );
}

bool is_allowed_event( User user, Event event )
{
	if ( registered_users.find( user ) == registered_users.cend() ) {
		return Event::user_registered == event;
	}

	if ( connected_users.find( user ) == connected_users.cend() ) {
		return Event::user_connected == event;
	}

	return Event::user_renamed == event || Event::user_deal_won == event || Event::user_disconnected == event;
}

template < typename Generator >
void writeEvent( std::ostream& outStream, User user, Event event, Generator& generator )
{
	outStream << event << " " << user;

	switch ( event ) {
		case Event::user_registered:
		case Event::user_renamed: {
			auto name = get_random_name( generator );
			outStream << " " << name;
			registered_users.emplace( user, name );
			break;
		}
		case Event::user_deal_won: {
			auto time = get_random_time( generator );
			outStream << " " << time.count() << " " << get_random_amount( generator );
			currentTime = time;
			break;
		}
		case Event::user_connected: {
			connected_users.emplace( user );
			break;
		}
		case Event::user_disconnected: {
			connected_users.erase( user );
			break;
		}
	}
	outStream << std::endl;
}

int main( int argc, char* argv[] )
{
	auto file_path = std::experimental::filesystem::path( "data.bin" );
	auto begin = std::chrono::steady_clock::now();
	std::ofstream file( file_path.c_str(), std::ios::binary | std::ios::out );
	if ( !file.is_open() ) {
		std::cerr << "Cannot open file data.txt." << std::endl;
		return -1;
	}

	using namespace std::chrono_literals;
	auto duration = ( argc > 1 ) ? std::chrono::hours( std::stoull( argv[ 1 ] ) ) : 8 * 24h;

	std::random_device random_device;
	std::mt19937 generator( random_device() );

	while ( currentTime < duration ) {
		auto user = get_random_user( generator );
		auto event = get_random_event( generator );
		while ( !is_allowed_event( user, event ) ) {
			event = get_random_event( generator );
		}

		writeEvent( file, user, event, generator );
	}
	auto end = std::chrono::steady_clock::now();

	std::cout << std::chrono::duration_cast< std::chrono::seconds >( end - begin ).count() << " "
	          << std::experimental::filesystem::file_size( file_path ) / 1024.0 << std::endl;

	return 0;
}
