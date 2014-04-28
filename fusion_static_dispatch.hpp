#pragma once

#include "index_list.hpp"
#include "friendly_fusion.hpp"

#include <boost/any.hpp>

#include <functional>

template <typename T, int index>
struct TypeAt {
	typedef friendly_fusion::result_of::begin<T> begin;
	typedef friendly_fusion::result_of::advance_c<typename begin::type, index> adv_it;
	typedef friendly_fusion::result_of::deref<typename adv_it::type> deref;
	typedef typename std::decay<typename deref::type>::type value_type;
};

#if defined( __GNUC__ ) && !defined( __clang__ )
template<int index, typename T>
std::function<boost::any(T)> make_at_c_lambda(T seq)
{
	return [](T seq){
		return boost::any(friendly_fusion::deref(friendly_fusion::advance_c<index>(friendly_fusion::begin(seq))));
	};
}
#endif //defined( __GNUC__ ) && !defined( __clang__ )

template<typename T, int... Indices>
boost::any get_nth_impl(T seq, int index, indices<Indices...>)
{
	typedef std::function<boost::any(T)> element_type;
	static element_type table[] =
	{
#if defined( __GNUC__ ) && !defined( __clang__ )
		//Workaround for gcc bug: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=47226
		make_at_c_lambda<Indices>(seq)
		...
#else
		[](T seq){return boost::any(friendly_fusion::deref(friendly_fusion::advance_c<Indices>(friendly_fusion::begin(seq))));}
		...
#endif //defined( __GNUC__ ) && !defined( __clang__ )
	};

	return table[index](seq);
}

template<typename T>
struct get_nth_functor
{
	boost::any operator()(T seq, int index)
	{
		typedef typename friendly_fusion::result_of::size<T>::type seq_size;
		typedef typename build_indices<seq_size::value>::type indices_type;
	
		return get_nth_impl(seq, index, indices_type{});
	}
};

template <typename T>
boost::any get_nth(T x, int index)
{
	return get_nth_functor<T>()(x, index);
}

#if defined( __GNUC__ ) && !defined( __clang__ )
template<int index, typename T>
std::function<std::string()> make_struct_member_name_lambda()
{
	return []{
		return std::string(friendly_fusion::extension::struct_member_name<T, index>::call());
	};
}
#endif //defined( __GNUC__ ) && !defined( __clang__ )

template<typename T>
struct get_nth_name_impl {
	
	get_nth_name_impl() = default;
	
	template<int... Indices>
	std::string operator()(int index, indices<Indices...>)
	{
		typedef std::function<std::string()> element_type;
		static element_type table[] = {
	#if defined( __GNUC__ ) && !defined( __clang__ )
			//Workaround for gcc bug: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=47226
			make_struct_member_name_lambda<Indices, T>()
			...
	#else
			[]{return std::string(friendly_fusion::extension::struct_member_name<T, Indices>::call());}
			...
	#endif //defined( __GNUC__ ) && !defined( __clang__ )
		};
	
		return table[index]();
	}
};

template <typename T>
struct get_nth_name_functor 
{
	std::string operator()(int index)
	{
		typedef typename friendly_fusion::result_of::size<T>::type seq_size;
		typedef typename build_indices<seq_size::value>::type indices_type;
	
		return get_nth_name_impl<T>()(index, indices_type{});
	}
};

template <typename T, typename U>
struct get_nth_name_functor<boost::fusion::joint_view<T,U>>
{
	std::string operator()(int index)
	{
		constexpr size_t size_of_T = friendly_fusion::result_of::size<T>::type::value;
		if(index < size_of_T){
			return get_nth_name_functor<T>()(index);
		} else {
			return get_nth_name_functor<U>()(index - size_of_T);
		}
	}
};

template <typename T>
std::string get_nth_name(int index)
{
	return get_nth_name_functor<T>()(index);
}
