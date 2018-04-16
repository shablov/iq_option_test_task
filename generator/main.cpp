#include <experimental/filesystem>
#include <fstream>
#include <iostream>

#include <libs/event_generator.h>

int main( int argc, char* argv[] )
{
	using namespace std::chrono_literals;
	auto duration = ( argc > 1 ) ? std::chrono::hours( std::stoull( argv[ 1 ] ) ) : 10 * 24h;
	const char* filename = ( argc > 2 ) ? "data.bin" : argv[ 2 ];

	auto begin = std::chrono::steady_clock::now();
	std::ofstream file( filename, std::ios::binary );
	if ( !file.is_open() ) {
		std::cerr << "Cannot open file " << filename << "." << std::endl;
		return -1;
	}

	std::chrono::nanoseconds currentTime = std::chrono::nanoseconds::zero();
	EventGenerator eventGenerator;
	while ( currentTime < duration ) {
		auto event = eventGenerator.generateEvent( currentTime );
		if ( Event::Type::user_deal_won == event->type() ) {
			UserDealWonEvent& dealWonEvent = static_cast< UserDealWonEvent& >( *event );
			currentTime = dealWonEvent.time();
		}
		file << *event << "\n";
	}
	file.close();
	auto end = std::chrono::steady_clock::now();

	std::cout << std::chrono::ceil< std::chrono::seconds >( end - begin ).count() << "s "
	          << std::experimental::filesystem::file_size( filename ) / 1024.0 << "Kb\n";

	return 0;
}
