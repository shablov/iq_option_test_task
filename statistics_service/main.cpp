#include <functional>
#include <sstream>
#include <thread>

#include <signal.h>
#include <unistd.h>

#include <libs/common.h>
#include <libs/event.h>

#include "events_handler.h"
#include "packets_handler.h"

static std::atomic< bool > stopped( false );

void receive_loop( const uint16_t receive_port, std::function< void( std::unique_ptr< Event >&& event ) > put_event )
{
	const auto socket_fd = createSocket();
	bindSocket( socket_fd, receive_port );

	constexpr size_t buffer_length = 1024;
	std::string buffer( buffer_length, '\0' );
	while ( !stopped.load() ) {
		if ( auto readBytes = recv( socket_fd, buffer.data(), buffer.size(), 0 ); readBytes <= 0 ) {
			continue;
		}

		std::istringstream ss( buffer );

		BaseEvent base_event;
		ss >> base_event;

		if ( auto event = Event::createEvent( base_event ); event ) {
			ss >> *event;
			put_event( std::move( event ) );
		}
	}
	close( socket_fd );
}

int main( int argc, char* argv[] )
{
	if ( argc < 4 ) {
		return -1;
	}

	const uint16_t receive_port = static_cast< uint16_t >( std::stoul( argv[ 1 ] ) );
	const char* send_address = argv[ 2 ];
	const uint16_t send_port = static_cast< uint16_t >( std::stoul( argv[ 3 ] ) );

	static PacketsHandler packets_handler( send_address, send_port );
	static EventsHandler events_handler( packets_handler );
	auto put_event = []( std::unique_ptr< Event >&& event ) { events_handler.put( std::move( event ) ); };

	auto stop_tasks = []( int ) {
		stopped.store( true );
		events_handler.stopProcessing();
		packets_handler.stopProcessing();
	};

	signal( SIGINT, stop_tasks );

	std::thread events_thread( [] { events_handler.procesing(); } );
	std::thread packets_thread( [] { packets_handler.proccesing(); } );

	std::thread receive_thread( std::bind( receive_loop, receive_port, put_event ) );

	receive_thread.join();
	events_thread.join();
	packets_thread.join();

	return 0;
}
