#pragma once

#include "friendly_fusion.hpp"

#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/vector.hpp>

#include <ostream>
#include <vector>
#include <string>

//Workaround for argument dependent lookup
namespace std
{

struct Outputter
{
	template <typename I, typename E>
	static typename std::enable_if<!std::is_same<I,E>::value, std::ostream&>::type
	output(std::ostream& os, I const& it, E const& end)
	{
		if(std::is_same<typename std::decay<typename friendly_fusion::result_of::deref<I>::type>::type, bool>::value) {
			os << std::boolalpha;
		}
		
		os << friendly_fusion::deref(it);
		
		if(!std::is_same<typename friendly_fusion::result_of::advance_c<I, 1>::type, E>::value) {
			os << ", ";
		}
		
		return Outputter::output(os, friendly_fusion::advance_c<1>(it), end);
	}

	template <typename I, typename E>
	static typename std::enable_if<std::is_same<I,E>::value, std::ostream&>::type
	output(std::ostream& os, I const&, E const&)
	{
		return os;
	}


};

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, std::map<T, U> map)
{
	os << "{" << std::endl;
	for (auto&& x : map) {
		os << "\t{" << x.first << ", " << x.second << "}" << std::endl;
	}
	os << "}" << std::endl;
	return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> vec)
{
	os << "{" << std::endl;
	for (auto&& x : vec) {
		os << "\t" << x << std::endl;
	}
	os << "}" << std::endl;
	return os;
}

template <typename T>
typename std::enable_if<friendly_fusion::traits::is_sequence<T>::value, std::ostream&>::type
operator<<(std::ostream& os, T x)
{
	return std::Outputter::output(os, friendly_fusion::begin(x), friendly_fusion::end(x));
}

}


