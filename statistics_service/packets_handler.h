#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <string_view>
#include <vector>

#include <arpa/inet.h>

#include "statistics.h"

struct Packet
{
	Event::User user;
	size_t position;
	SortedStatistic top;
	SortedStatistic near;

	friend std::ostream& operator<<( std::ostream& out, const Packet& packet );
};

class PacketsHandler
{
public:
	PacketsHandler( const std::string_view address, const uint16_t port );
	~PacketsHandler();

	void put( std::vector< Packet >&& packets );
	void put( Packet&& packet );

	void proccesing();

	void stopProcessing();

private:
	bool pop();

	void send( const std::string_view data );

	std::atomic< bool > _stopped;
	std::mutex _mutex;
	std::condition_variable _condition_variable;

	std::deque< Packet > _unhandled_packets;
	Packet _processing_packet;

	const int _socket;
	const struct sockaddr_in _sockaddr;
};
