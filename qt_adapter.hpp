#pragma once

#include "fusion_model.hpp"
#include "boost_any_qvariant_convert.hpp"
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
//	typedef void type;
};

template <typename T>
struct widget_type<std::shared_ptr<T>> {
	typedef typename T::widget type;
};

template <typename T>
struct widget_type<std::shared_ptr<T>&&>
{
	typedef typename T::widget type;
};

template <typename T>
struct widget_type<std::shared_ptr<T>&>
{
	typedef typename T::widget type;
};

template <typename T, typename U>
struct QtWidget : public T {
	
	GuiItemDelegate delegate;
	std::shared_ptr<U> model;
	
	QtWidget(std::shared_ptr<U> model, QWidget* parent = nullptr) 
	: T(parent)
	, delegate()
	, model(model)
	{
		T::setModel(this->model.get());
		T::setItemDelegate(&delegate);
		T::setEditTriggers(QAbstractItemView::DoubleClicked);
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
	typedef QtWidget<view, QtAdapter<T, QAbstractTableModel>> widget;
	
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
	
	bool setData(QModelIndex const& index, QVariant const& value, int role) override final
	{
		if(role != Qt::EditRole) return false;
		
		model.set_cell(index.row(), index.column(), to_boost_any<typename T::row_type>(value, index.column()));
		
		return true;
	}
	
	virtual Qt::ItemFlags flags(const QModelIndex &index) const
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
	
	virtual QVariant data(QModelIndex const& index, int role) const override
	{
		if(role != Qt::DisplayRole && role != Qt::EditRole) return QVariant();
		
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
std::shared_ptr<QtAdapter<T, typename QtModelType<typename T::data_type>::type>> make_qt_adapter(T value) {
	typedef QtAdapter<T, typename QtModelType<typename T::data_type>::type> type;
	return std::make_shared<type>(value);
}
