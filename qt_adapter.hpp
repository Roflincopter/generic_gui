#pragma once

#include "fusion_model.hpp"
#include "boost_any_to_qvariant.hpp"
#include "meta_types.hpp"

#include <QAbstractTableModel>
#include <QTableView>
#include <QMetaType>

#include <memory>



template <typename T>
struct QtModelType
{
	typedef void type;
};

template <typename V>
struct QtModelType<std::vector<V>>
{
	typedef QAbstractTableModel type;
};

template <typename T, typename Q>
struct QtAdapter : public Q {
	
	typedef void view;
	
	QtAdapter(T)
	{}
};

template <typename T>
struct QtAdapter<T, QAbstractTableModel> : public QAbstractTableModel
{
	typedef QTableView view;
	
	T model;
	
	QtAdapter(T model)
	: model(model)
	{}
	
	virtual int rowCount(QModelIndex const&) const override
	{
		return model.row_count();
	}
	
	virtual int columnCount(QModelIndex const&) const override
	{
		return model.column_count();
	}
	
	virtual QVariant data(QModelIndex const& index, int role) const
	{
		if(role != Qt::DisplayRole) return QVariant();
		
		return to_qvariant<typename T::data_type::value_type>(model.get_cell(index.row(), index.column()), index.column());
	}
};


template <typename T>
std::shared_ptr<QtAdapter<T, typename QtModelType<typename T::data_type>::type>> make_qt_adapter(T value) {
	return std::make_shared<QtAdapter<T, typename QtModelType<typename T::data_type>::type>>(value);
}
