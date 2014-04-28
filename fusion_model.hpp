#pragma once

#include "fusion_static_dispatch.hpp"

#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/size.hpp>

#include <boost/any.hpp>

#include <vector>
#include <map>
#include <iostream>

template <typename T>
struct fusion_model {
private:
	fusion_model(T);
};

struct FusionModelInterface {
	virtual size_t row_count() const {throw std::runtime_error("\"row_count()\" not implemented for this model");}
	virtual size_t column_count() const {throw std::runtime_error("\"column_count()\" not implemented for this model");}
	virtual std::string field_name(size_t section) const {throw std::runtime_error("\"field_name(size_t)\" not implemented for this model");}
	virtual std::string key(size_t section) const {throw std::runtime_error("\"key(size_t)\" not implemented for this model");}
	virtual boost::any get_cell(size_t row, size_t column) const {throw std::runtime_error("\"get_cell(size_t, size_t)\" not implemented for this model");}
};

template <typename T>
struct fusion_model<std::vector<T>> : public FusionModelInterface
{
	static_assert(friendly_fusion::traits::is_sequence<T>::type::value, "T is not a boost fusion sequence");
	
	typedef std::vector<T> data_type;
	typedef T row_type;
	
	static constexpr bool has_header_h = true;
	static constexpr bool has_header_v = false;
	
	std::vector<T> data;
	
	virtual size_t row_count() const override final
	{
		return data.size();
	}
	
	virtual size_t column_count() const override final
	{
		return boost::fusion::result_of::size<T>::type::value;
	}
	
	virtual std::string field_name(size_t section) const override final
	{
		return get_nth_name<T>(section);
	}
	
	virtual boost::any get_cell(size_t row, size_t column) const override final
	{
		return get_nth(data[row], column);
	}
};

template <typename T>
struct fusion_model<std::map<std::string, T>> : public FusionModelInterface
{
	static_assert(boost::fusion::traits::is_sequence<T>::type::value, "T is not a boost fusion sequence");
	
	typedef std::map<std::string, T> data_type;
	typedef T row_type;
	
	static constexpr bool has_header_h = true;
	static constexpr bool has_header_v = true;
	
	std::map<std::string, T> data;
	
	virtual size_t row_count() const override final
	{
		return data.size();
	}
	
	virtual size_t column_count() const override final
	{
		return boost::fusion::result_of::size<T>::type::value;
	}
	
	virtual std::string field_name(size_t section) const override final
	{
		return get_nth_name<T>(section);
	}
	
	virtual std::string key(size_t section) const override final
	{
		auto cit = data.cbegin();
		std::advance(cit, section);
		return cit->first;
	}
	
	virtual boost::any get_cell(size_t row, size_t column) const override final
	{
		auto cit = data.cbegin();
		std::advance(cit, row);
		return get_nth(cit->second, column);
	}
};