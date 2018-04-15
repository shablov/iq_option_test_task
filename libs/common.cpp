#include "common.h"

int createSocket()
{
	return socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
}

struct sockaddr_in getSelfSockaddr(const uint16_t port )
{
	struct sockaddr_in self{};
	self.sin_family = AF_INET;
	self.sin_port = htons( port );
	self.sin_addr.s_addr = htonl( INADDR_ANY );
	return self;
}

struct sockaddr_in getRemoteSockaddr(const std::string_view address, const uint16_t port )
{
	struct sockaddr_in remote{};
	remote.sin_family = AF_INET;
	remote.sin_port = htons( port );
	inet_aton( address.data(), &remote.sin_addr );

	return remote;
}

void bindSocket(const int socket, const uint16_t port )
{
	const auto self = getSelfSockaddr( port );

	bind( socket, reinterpret_cast< const struct sockaddr* >( &self ), sizeof( self ) );
}
