#include <atomic>
#include <fstream>
#include <iostream>
#include <thread>

#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <libs/common.h>

int main( int argc, char* argv[] )
{
	if ( argc != 5 ) {
		return -1;
	}

	static std::atomic< bool > stopped( false );

	auto stop_tasks = []( int ) { stopped.store( true ); };

	signal( SIGINT, stop_tasks );

	uint16_t receive_port = static_cast< uint16_t >( std::stoul( argv[ 1 ] ) );
	std::thread receive_thread( [receive_port]() {
		auto socket_fd = createSocket();
		bindSocket( socket_fd, receive_port );

		constexpr size_t buffer_length = 4096;
		std::string buffer;
		while ( !stopped.load() ) {
			buffer.resize( buffer_length );
			auto readBytes = recv( socket_fd, buffer.data(), buffer.size(), 0 );
			if ( readBytes <= 0 ) {
				continue;
			}
			buffer.resize( static_cast< size_t >( readBytes ) );

			std::cout << buffer << std::endl;
		}

		close( socket_fd );
	} );

	uint16_t send_port = static_cast< uint16_t >( std::stoul( argv[ 3 ] ) );
	std::thread send_thread( [filename = argv[ 4 ], address = argv[ 2 ], send_port]() {
		auto socket_fd = createSocket();
		auto sockaddr = getRemoteSockaddr( address, send_port );

		std::ifstream in_file( filename, std::ios::binary );
		while ( !in_file.eof() && !stopped.load() ) {
			std::string str;
			std::getline( in_file, str );

			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1us);

			sendto( socket_fd,
			        str.data(),
			        str.size(),
			        0,
			        reinterpret_cast< struct sockaddr* >( &sockaddr ),
			        sizeof( sockaddr ) );
		}

		close( socket_fd );
	} );

	send_thread.join();
	receive_thread.join();

	return 0;
}
