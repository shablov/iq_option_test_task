#include "common.h"

int createSocket()
{
	return socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
}

struct sockaddr_in getSelfSockaddr( uint16_t port )
{
	struct sockaddr_in self;
	std::memset( &self, sizeof( self ), 0x00 );
	self.sin_family = AF_INET;
	self.sin_port = htons( port );
	self.sin_addr.s_addr = htonl( INADDR_ANY );
	return self;
}

struct sockaddr_in getRemoteSockaddr( std::string_view address, uint16_t port )
{
	struct sockaddr_in _sockaddr;
	std::memset( &_sockaddr, sizeof( _sockaddr ), 0x00 );
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_port = htons( port );
	inet_aton( address.data(), &_sockaddr.sin_addr );

	return _sockaddr;
}

void bindSocket( int socket, uint16_t port )
{
	struct sockaddr_in self = getSelfSockaddr( port );

	bind( socket, reinterpret_cast< struct sockaddr* >( &self ), sizeof( self ) );
}
