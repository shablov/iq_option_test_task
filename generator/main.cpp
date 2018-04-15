#include <chrono>
#include <cstddef>
#include <deque>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <libs/event.h>

using User = Event::User;

std::chrono::nanoseconds currentTime = std::chrono::nanoseconds::zero();

std::unordered_map< User, std::string > registered_users;
std::unordered_set< User > connected_users;

template < typename Generator >
auto get_random_user( Generator& generator )
{
	static constexpr User min_user_id = User( 0 );
	static constexpr User max_user_id = 1000;

	static std::uniform_int_distribution distribution( min_user_id, max_user_id );

	return distribution( generator );
}

template < typename Generator >
auto get_random_event( Generator& generator )
{
	using enum_type = std::underlying_type_t< Event::Type >;
	static constexpr auto lowest_event = static_cast< enum_type >( Event::Type::user_registered );
	static constexpr auto highest_event = static_cast< enum_type >( Event::Type::user_disconnected );

	static std::uniform_int_distribution distribution( lowest_event, highest_event );

	return static_cast< Event::Type >( distribution( generator ) );
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
	static constexpr int32_t min_amount = -1000;
	static constexpr int32_t max_amount = +1000;

	static std::uniform_int_distribution distribution( min_amount, max_amount );

	return distribution( generator );
}

bool is_allowed_event( const Event& event )
{
	if ( registered_users.find( event.user() ) == registered_users.cend() ) {
		return Event::Type::user_registered == event.type();
	}

	if ( connected_users.find( event.user() ) == connected_users.cend() ) {
		return Event::Type::user_connected == event.type();
	}

	return Event::Type::user_renamed == event.type() || Event::Type::user_deal_won == event.type()
	       || Event::Type::user_disconnected == event.type();
}

template < typename Generator >
void writeEvent( std::ostream& outStream, const Event& baseEvent, Generator& generator )
{
	std::unique_ptr< Event > event = Event::createEvent( baseEvent );
	switch ( baseEvent.type() ) {
		case Event::Type::user_registered: {
			auto name = get_random_name( generator );
			event.reset( new UserRegisteredEvent( baseEvent.user(), name ) );
			registered_users.emplace( baseEvent.user(), name );
			break;
		}
		case Event::Type::user_renamed: {
			auto name = get_random_name( generator );
			event.reset( new UserRenamedEvent( baseEvent.user(), name ) );
			registered_users.emplace( baseEvent.user(), name );
			break;
		}
		case Event::Type::user_deal_won: {
			auto time = get_random_time( generator );
			auto amount = get_random_amount( generator );

			event.reset( new UserDealWonEvent( baseEvent.user(), time, amount ) );

			currentTime = time;
			break;
		}
		case Event::Type::user_connected: {
			event.reset( new UserConnectedEvent( baseEvent.user() ) );
			connected_users.emplace( baseEvent.user() );
			break;
		}
		case Event::Type::user_disconnected: {
			event.reset( new UserDisconnectedEvent( baseEvent.user() ) );
			connected_users.erase( baseEvent.user() );
			break;
		}
		case Event::Type::undefined: {
			break;
		}
	}
	outStream << *event << "\n";
}

int main( int argc, char* argv[] )
{
	auto filename = "data.bin";

	auto begin = std::chrono::steady_clock::now();
	std::ofstream file( filename, std::ios::binary );
	if ( !file.is_open() ) {
		std::cerr << "Cannot open file " << filename << "." << std::endl;
		return -1;
	}

	using namespace std::chrono_literals;
	auto duration = ( argc > 1 ) ? std::chrono::hours( std::stoull( argv[ 1 ] ) ) : 10 * 24h;

	std::random_device random_device;
	std::mt19937 generator( random_device() );

	while ( currentTime < duration ) {
		BaseEvent event( get_random_user( generator ), get_random_event( generator ) );
		while ( !is_allowed_event( event ) ) {
			event.setType( get_random_event( generator ) );
		}

		writeEvent( file, event, generator );
	}
	file.close();
	auto end = std::chrono::steady_clock::now();

	std::cout << std::chrono::duration_cast< std::chrono::seconds >( end - begin ).count() << " "
	          << std::experimental::filesystem::file_size( filename ) / 1024.0 << "\n";

	return 0;
}
