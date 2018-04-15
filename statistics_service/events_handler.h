#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <libs/event.h>

#include "packets_handler.h"
#include "statistics.h"

class EventsHandler
{
public:
	EventsHandler( PacketsHandler& packetsHandler );

	void put( std::unique_ptr< Event >&& event );

	void proccesing();

	void stopProcessing();

	void printAll()
	{
		std::cout << "print_all" << std::endl;
		for ( auto& e : _sorted_statistics ) {
			std::cout << "user_id: " << e.second << " amount: " << e.first << "\n";
		}
	}

private:
	bool pop();

	void registered( const UserRegisteredEvent& event );
	void connected( const UserConnectedEvent& event );
	void renamed( const UserRenamedEvent& event );
	void dealWon( const UserDealWonEvent& event );
	void disconnected( const UserDisconnectedEvent& event );

	void addNewUser( Event::User user, std::string_view name );

	void sendUserStatistics( Event::User user );

	Packet getUserStatistics( Event::User user );

	void sendPacket(Packet&& packet );

	auto getUserRank( Event::User user );

	void updateUserStatistics( Event::User user, int64_t amount, std::chrono::nanoseconds time );

	bool isNextMinute( std::chrono::nanoseconds time ) const;

	bool isNewWeek( std::chrono::nanoseconds time ) const;

	void clearStatistics();

	void sendPackets();

	void updateTime( std::chrono::nanoseconds time );

	std::atomic< bool > _stopped;
	std::mutex _mutex;
	std::condition_variable _condition_variable;

	std::deque< std::unique_ptr< Event > > _unhandled_events;
	std::vector< std::unique_ptr< Event > > _processing_events;

	std::unordered_map< Event::User, std::string > _registered_users;

	std::chrono::nanoseconds last_update_time;
	std::unordered_set< Event::User > _connected_users;
	Statistics _statistics;
	SortedStatistic _sorted_statistics;

	PacketsHandler& _packets_handler;
};
