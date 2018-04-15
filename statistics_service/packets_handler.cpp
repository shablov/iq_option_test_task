#include "packets_handler.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

#include <unistd.h>

#include <libs/common.h>

std::ostream& operator<<( std::ostream& out, const Packet& packet )
{
	auto add_user_to_packet = [&out]( const auto& value ) {
		const auto [ id, amount ] = value;
		out << "\tid: " << id << " amount: " << amount << "\n";
	};

	out << "id: " << packet.user << " position: " << packet.position << "\n";
	out << "top: \n";
	std::for_each( packet.top.cbegin(), packet.top.cend(), add_user_to_packet );

	out << "near: \n";
	std::for_each( packet.near.cbegin(), packet.near.cend(), add_user_to_packet );

	return out;
}

PacketsHandler::PacketsHandler( const std::string_view address, const uint16_t port )
		: _socket( createSocket() ), _sockaddr( getRemoteSockaddr( address, port ) )
{
}

PacketsHandler::~PacketsHandler()
{
	close( _socket );
}

void PacketsHandler::put( std::vector< Packet >&& packets )
{
	std::unique_lock lock( _mutex );
	_unhandled_packets.assign( std::move_iterator( packets.begin() ), std::move_iterator( packets.end() ) );
	_condition_variable.notify_one();
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
		std::ostringstream ss;
		ss << _processing_packet;

		send( ss.str() );
	}
}

void PacketsHandler::stopProcessing()
{
	_stopped.store( true );
	_condition_variable.notify_one();
}

bool PacketsHandler::pop()
{
	std::unique_lock lock( _mutex );
	_condition_variable.wait( lock, [this] { return _stopped.load() || !_unhandled_packets.empty(); } );

	if ( !_stopped.load() ) {
		_processing_packet = std::move( _unhandled_packets.front() );
		_unhandled_packets.pop_front();
	}

	return !_stopped.load();
}

void PacketsHandler::send( const std::string_view data )
{
	sendto( _socket, data.data(), data.size(), 0, reinterpret_cast< const struct sockaddr* >( &_sockaddr ), sizeof( _sockaddr ) );
}
