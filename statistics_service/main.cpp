#include <fstream>
#include <sstream>
#include <thread>

#include <signal.h>

#include <libs/common.h>
#include <libs/event.h>

#include "events_handler.h"
#include "packets_handler.h"

int main( int argc, char* argv[] )
{
	if ( argc != 4 ) {
		return -1;
	}

	static std::atomic< bool > stopped(false);
	static PacketsHandler packets_handler( argv[ 2 ], static_cast< uint16_t >( std::stoul( argv[ 3 ] ) ) );
	static EventsHandler events_handler( packets_handler );

	auto stop_tasks = []( int ) {
		stopped.store( true );
		events_handler.stopProcessing();
		packets_handler.stopProcessing();
	};

	signal( SIGINT, stop_tasks );

	std::thread events_thread( []() { events_handler.proccesing(); } );
	std::thread packets_thread( []() { packets_handler.proccesing(); } );

	std::thread receive_thread( [port = argv[ 1 ]]() {
		auto _socket = createSocket();
		bindSocket( _socket, static_cast< uint16_t >( std::stoul( port ) ) );

		constexpr size_t buffer_length = 1024;
		std::string buffer;
		while ( !stopped.load() ) {
			buffer.resize( buffer_length );
			auto readBytes = recv( _socket, buffer.data(), buffer.size(), 0 );
			if ( readBytes <= 0 ) {
				continue;
			}
			buffer.resize( static_cast< size_t >( readBytes ) );

			std::istringstream ss( buffer );

			BaseEvent base_event;
			ss >> base_event;

			auto event = Event::createEvent( base_event );
			if ( event ) {
				ss >> *event;
				events_handler.put( std::move( event ) );
			}
		}
		close( _socket );

		events_handler.stopProcessing();
		packets_handler.stopProcessing();

		return 0;
	} );

	receive_thread.join();
	events_thread.join();
	packets_thread.join();

	events_handler.printAll();

	return 0;
}
