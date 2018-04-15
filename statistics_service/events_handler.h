#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string_view>
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

	void procesing();

	void stopProcessing();

private:
	static constexpr int neighbors_count = 10;

	bool pop();

	void registered( const UserRegisteredEvent& event );
	void connected( const UserConnectedEvent& event );
	void renamed( const UserRenamedEvent& event );
	void dealWon( const UserDealWonEvent& event );
	void disconnected( const UserDisconnectedEvent& event );

	void addNewUser( const Event::User user, const std::string_view name );

	void sendUserStatistics( const Event::User user );

	Packet userStatistic( const Event::User user ) const;

	SortedStatistic::const_iterator userRank( const Event::User user ) const;

	SortedStatistic topStatistic() const;
	SortedStatistic neigborsStatistic( const SortedStatistic::const_iterator rank, const size_t position ) const;

	void sendPacket( Packet&& packet );

	void updateUserStatistics( const Event::User user, const int64_t amount, const std::chrono::nanoseconds time );

	bool isNewWeek( const std::chrono::nanoseconds time ) const noexcept;

	void clearStatistics();

	bool isNextMinute( const std::chrono::nanoseconds time ) const noexcept;

	void addUserAmount( const Event::User user, const int64_t amount );

	void sendPackets();

	void updateTime( const std::chrono::nanoseconds time ) noexcept;

	std::atomic< bool > _stopped;
	std::mutex _mutex;
	std::condition_variable _condition_variable;

	std::deque< std::unique_ptr< Event > > _unhandled_events;
	std::vector< std::unique_ptr< Event > > _processing_events;

	std::unordered_map< Event::User, std::string > _registered_users;

	std::chrono::nanoseconds last_update_week_time;
	std::unordered_set< Event::User > _connected_users;
	Statistics _statistics;
	SortedStatistic _sorted_statistics;

	PacketsHandler& _packets_handler;
};
