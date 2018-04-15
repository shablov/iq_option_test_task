#pragma once

#include <cstddef>
#include <string_view>

#include <arpa/inet.h>

int createSocket();

struct sockaddr_in getSelfSockaddr( const uint16_t port );

struct sockaddr_in getRemoteSockaddr( const std::string_view address, const uint16_t port );

void bindSocket( const int socket, const uint16_t port );
