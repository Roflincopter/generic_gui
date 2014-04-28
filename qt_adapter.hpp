#pragma once

#include "fusion_model.hpp"
#include "boost_any_to_qvariant.hpp"
#include "meta_types.hpp"
#include "gui_item_delegate.hpp"

#include <QAbstractTableModel>
#include <QTableView>
#include <QVariant>
#include <QMetaType>

#include <memory>
#include <string>
#include <iostream>

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

template <typename V>
struct QtModelType<std::map<std::string, V>>
{
	typedef QAbstractTableModel type;
};

template <typename T>
struct widget_type {
	typedef void type;
};

template <typename T>
struct widget_type<std::unique_ptr<T>> {
	typedef typename T::widget type;
};

template <typename T>
struct QtWidget : public T {
	
	GuiItemDelegate delegate;
	
	QtWidget(QWidget* parent = nullptr) 
	: T(parent)
	{
		T::setItemDelegate(&delegate);
	}
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
	typedef QtWidget<view> widget;
	
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
	
	virtual QVariant data(QModelIndex const& index, int role) const override
	{
		if(role != Qt::DisplayRole) return QVariant();
		
		return to_qvariant<typename T::row_type>(model.get_cell(index.row(), index.column()), index.column());
	}
	
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override
	{	
		if(role != Qt::DisplayRole) {
			return QVariant();
		}
		
		if(orientation == Qt::Horizontal && T::has_header_h) {
			return QVariant(QString::fromStdString(model.field_name(section)));
		}
		
		if(orientation == Qt::Vertical && T::has_header_v) {
			return QVariant(QString::fromStdString(model.key(section)));
		}
		
		return QVariant();
	}
};

template <typename T>
std::unique_ptr<QtAdapter<T, typename QtModelType<typename T::data_type>::type>> make_qt_adapter(T value) {
	typedef QtAdapter<T, typename QtModelType<typename T::data_type>::type> type;
	return std::unique_ptr<type>(new type(value));
}
