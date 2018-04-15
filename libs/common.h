#pragma once

#include <cstddef>
#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int createSocket();

struct sockaddr_in getSelfSockaddr( uint16_t port );

struct sockaddr_in getRemoteSockaddr( std::string_view address, uint16_t port );

void bindSocket( int socket, uint16_t port );
