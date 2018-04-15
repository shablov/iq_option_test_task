#include "packets_handler.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

#include <libs/common.h>

PacketsHandler::PacketsHandler( std::string_view address, uint16_t port )
{
	_socket_fd = createSocket();

	_sockaddr = getRemoteSockaddr( address, port );
}

PacketsHandler::~PacketsHandler()
{
	close( _socket_fd );
}

void PacketsHandler::replace( std::vector< Packet >&& packets )
{
	std::unique_lock lock( _mutex );
	if ( _unhandled_packets.size() > 0 ) {
		std::cout << _unhandled_packets.size() << std::endl;
	}
	_unhandled_packets.assign( std::move_iterator( packets.begin() ), std::move_iterator( packets.end() ) );
}

void PacketsHandler::put( Packet&& packet )
{
	std::unique_lock lock( _mutex );
	_unhandled_packets.push_back( std::move( packet ) );
	_condition_variable.notify_one();
}

void PacketsHandler::proccesing()
{
	_stopped.store( false );
	while ( pop() ) {
		for ( const auto& packet : _processing_packets ) {
			std::ostringstream ss;
			auto add_user_to_packet = [&ss]( const auto& status ) {
				ss << "\tid: " << status.second << " amount: " << status.first << "\n";
			};

			ss << "id: " << packet.user << " position: " << packet.position << "\n";
			ss << "top: \n";
			std::for_each( packet.top.cbegin(), packet.top.cend(), add_user_to_packet );

			ss << "near: \n";
			std::for_each( packet.near.cbegin(), packet.near.cend(), add_user_to_packet );

			//			std::cout << ss.str() << std::endl;

			const auto& str = ss.str();
			sendto( _socket_fd,
			        str.data(),
			        str.size(),
			        0,
			        reinterpret_cast< struct sockaddr* >( &_sockaddr ),
			        sizeof( sockaddr ) );
		}
	}
}

void PacketsHandler::stopProcessing()
{
	_stopped.store( true );
	_condition_variable.notify_one();
}

bool PacketsHandler::pop()
{
	_processing_packets.clear();
	std::unique_lock lock( _mutex );
	_condition_variable.wait( lock, [this]() { return !( !_stopped.load() && _unhandled_packets.empty() ); } );

	//	_processing_packets.assign( std::move_iterator( _unhandled_packets.begin() ),
	//	                            std::move_iterator( _unhandled_packets.end() ) );
	//	_unhandled_packets.clear();
	_processing_packets.push_back( std::move( _unhandled_packets.front() ) );
	_unhandled_packets.pop_front();

	return !_stopped.load();
}
