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

template <bool header_h, bool header_v>
struct FusionModelInterface {
	
	static constexpr bool has_header_h = header_h;
	static constexpr bool has_header_v = header_v;
	
	virtual size_t row_count() const {throw std::runtime_error("\"row_count()\" not implemented for this model");}
	virtual size_t column_count() const {throw std::runtime_error("\"column_count()\" not implemented for this model");}
	virtual std::string field_name(size_t section) const {throw std::runtime_error("\"field_name(size_t)\" not implemented for this model");}
	virtual std::string key(size_t section) const {throw std::runtime_error("\"key(size_t)\" not implemented for this model");}
	virtual boost::any get_cell(size_t row, size_t column) const {throw std::runtime_error("\"get_cell(size_t, size_t)\" not implemented for this model");}
	virtual void set_cell(size_t row, size_t column, boost::any const& value) {throw std::runtime_error("\"set_cell(size_t, size_t, boost::any const&)\" not implemented for this model");}
};

template <typename T>
struct fusion_model<std::vector<T>> : public FusionModelInterface<true, false>
{
	static_assert(friendly_fusion::traits::is_sequence<T>::type::value, "T is not a boost fusion sequence");
	
	typedef std::vector<T> data_type;
	typedef T row_type;
	
	std::vector<T> data;
	
	fusion_model() = default;
	
	fusion_model(std::vector<T> data)
	: data(data)
	{}
	
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
	
	virtual void set_cell(size_t row, size_t column, boost::any const& value) override final
	{
		set_nth<row_type>(data[row], column, value);
	}
};

template <typename T>
struct fusion_model<std::map<std::string, T>> : public FusionModelInterface<true, true>
{
	static_assert(boost::fusion::traits::is_sequence<T>::type::value, "T is not a boost fusion sequence");
	
	typedef std::map<std::string, T> data_type;
	typedef T row_type;
	
	std::map<std::string, T> data;
	
	fusion_model() = default;
	
	fusion_model(std::map<std::string, T> data)
	: data(data)
	{}
	
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
	
	virtual void set_cell(size_t row, size_t column, boost::any const& value) override final
	{
		auto it = data.begin();
		std::advance(it, row);
		set_nth<row_type>(it->second, column, value);
	}
};