#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <vector>

#include <arpa/inet.h>

#include "statistics.h"

struct Packet
{
	Event::User user;
	int64_t position;
	SortedStatistic top;
	SortedStatistic near;
};

class PacketsHandler
{
public:
	PacketsHandler( std::string_view address, uint16_t port );
	~PacketsHandler();

	void replace( std::vector< Packet >&& packets );
	void put( Packet&& packet );

	void proccesing();

	void stopProcessing();

private:
	bool pop();

	void send( const std::string& data );

	std::atomic< bool > _stopped;
	std::mutex _mutex;
	std::condition_variable _condition_variable;

	std::deque< Packet > _unhandled_packets;
	std::vector< Packet > _processing_packets;

	int _socket_fd;
	struct sockaddr_in _sockaddr;
};
