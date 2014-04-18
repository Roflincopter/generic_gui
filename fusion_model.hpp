#pragma once

#include "fusion_static_dispatch.hpp"

#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/size.hpp>

#include <boost/any.hpp>

#include <vector>
#include <map>

template <typename T>
struct fusion_model {
	
};

template <typename T>
struct fusion_model<std::vector<T>>
{
	static_assert(boost::fusion::traits::is_sequence<T>::type::value, "T is not a boost fusion sequence");
	
	typedef std::vector<T> data_type;
	
	std::vector<T> data;
	
	size_t row_count() const
	{
		return data.size();
	}
	
	size_t column_count() const
	{
		return boost::fusion::result_of::size<T>::type::value;
	}
	
	std::string horizontal_header_data(size_t section) const
	{
		return get_nth_name<T>(section);
	}
	
	boost::any get_cell(size_t row, size_t column) const 
	{
		return get_nth(data[row], column);
	}
};

template <typename T, typename U>
struct fusion_model<std::map<T, U>>
{
	std::map<T, U> data;
};