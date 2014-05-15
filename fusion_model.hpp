#pragma once

#include "fusion_static_dispatch.hpp"

#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/size.hpp>

#include <boost/any.hpp>

#include <vector>
#include <map>
#include <memory>

struct FusionModelObserver
{
	virtual void cell_changed(int row, int column) {}
	
	virtual void append_row_begin() {}
	virtual void append_row_end() {}
};

template <typename T>
struct FusionModel {
	FusionModel(T) = delete;
};

template <bool>
struct FusionModelWithHeaderH;

template <>
struct FusionModelWithHeaderH<false>
{
	static constexpr bool has_header_h = false;
};

template <>
struct FusionModelWithHeaderH<true>
{
	static constexpr bool has_header_h = true;
	
	virtual std::string field_name(size_t section) const = 0;
};

template <bool>
struct FusionModelWithHeaderV;

template <>
struct FusionModelWithHeaderV<false>
{
	static constexpr bool has_header_v = false;
};

template <>
struct FusionModelWithHeaderV<true>
{
	static constexpr bool has_header_v = true;
	
	virtual std::string key(size_t section) const = 0;
};

template <bool has_header_h, bool has_header_v>
struct FusionModelInterface : public FusionModelWithHeaderH<has_header_h>, public FusionModelWithHeaderV<has_header_v> {
	virtual size_t row_count() const = 0;
	virtual size_t column_count() const = 0;
	virtual boost::any get_cell(size_t row, size_t column) const = 0;
	virtual void set_cell(size_t row, size_t column, boost::any const& value) = 0;
	
	std::vector<std::weak_ptr<FusionModelObserver>> observers;
	void add_observer(std::weak_ptr<FusionModelObserver> obs)
	{
		observers.push_back(obs);
	}
	
	template <typename U, typename... Args>
	void call_on_observers(U f, Args... arguments)
	{
		for(auto it = observers.begin(); it != observers.end(); ++it)
		{
			auto s = it->lock();
			if(s) {
				((*s).*f)(arguments...);
			} else {
				it = observers.erase(it);
			}
		}
	}
};

template <typename T>
struct FusionModel<std::vector<T>> : public FusionModelInterface<true, false>
{
	static_assert(friendly_fusion::traits::is_sequence<T>::type::value, "T is not a boost fusion sequence");
	
	typedef std::vector<T> data_type;
	typedef T row_type;
	
	std::vector<T> data;
	
	FusionModel() = default;
	
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
		call_on_observers(&FusionModelObserver::cell_changed, row, column);
	}
};

template <typename T>
struct FusionModel<std::map<std::string, T>> : public FusionModelInterface<true, true>
{
	static_assert(boost::fusion::traits::is_sequence<T>::type::value, "T is not a boost fusion sequence");
	
	typedef std::map<std::string, T> data_type;
	typedef T row_type;
	
	std::map<std::string, T> data;
	
	FusionModel() = default;
	
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