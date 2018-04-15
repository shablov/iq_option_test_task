#pragma once

#include <cstddef>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <libs/event.h>

struct StatisticsComparator
{
	using value_type = std::pair< int64_t, Event::User >;

	bool operator()( const value_type& lhs, const value_type& rhs ) const
	{
		const auto [ lhs_amount, lhs_id ] = lhs;
		const auto [ rhs_amount, rhs_id ] = rhs;
		if ( lhs_amount == rhs_amount ) {
			return lhs_id < rhs_id;
		}
		return lhs_amount > rhs_amount;
	}
};

using Statistics = std::unordered_map< Event::User, int64_t >;
using SortedStatistic = std::set< std::pair< int64_t, Event::User >, StatisticsComparator >;
