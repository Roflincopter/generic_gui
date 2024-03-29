#pragma once

#include "fusion_model.hpp"
#include "boost_any_qvariant_convert.hpp"
#include "meta_types.hpp"
#include "gui_item_delegate.hpp"
#include "form.hpp"

#include <QAbstractTableModel>
#include <QTableView>
#include <QListView>
#include <QVariant>

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <map>

namespace Gui
{
	struct Table{};
	struct Form{};
}

template <typename T>
struct QtModelType
{
	typedef void type;
};

template <bool singular>
struct QtVectorModels;

template <>
struct QtVectorModels<true>
{
	typedef QAbstractListModel type;
};

template <>
struct QtVectorModels<false>
{
	typedef QAbstractTableModel type;
};

template <typename V>
struct QtModelType<std::vector<V>>
{
	typedef typename QtVectorModels<
		friendly_fusion::result_of::size<V>::type::value == 1
	>::type type;
};

template <typename V>
struct QtModelType<std::map<std::string, V>>
{
	typedef QAbstractTableModel type;
};

template <typename T, typename Q>
struct QtAdapter : public Q {
	
	typedef void view;
	
	QtAdapter(std::shared_ptr<T>)
	{}
};

template <typename T>
struct AdapterType;

template <typename T, typename Model>
struct QtWidget : public T {
	GuiItemDelegate delegate;
	std::shared_ptr<typename AdapterType<Model>::type> model;
	
	QtWidget(std::shared_ptr<Model> model, QWidget* parent = nullptr) 
	: T(parent)
	, delegate()
	, model(std::make_shared<typename AdapterType<Model>::type>(model))
	{
		model->add_observer(this->model);
		T::setModel(this->model.get());
		T::setItemDelegate(&delegate);
		T::setEditTriggers(QAbstractItemView::DoubleClicked);
	}
};

template <typename T, typename Model>
struct QtAdapterBase : public Model, public FusionModelObserver
{
	std::shared_ptr<T> model;
	
	QtAdapterBase(std::shared_ptr<T> model)
	: model(model)
	{}
	
	//ModelInterface
	
	virtual int columnCount(QModelIndex const&) const override
	{
		return model->column_count();
	}
	
	virtual int rowCount(QModelIndex const&) const override
	{
		return model->row_count();
	}
	
	bool setData(QModelIndex const& index, QVariant const& value, int role) override final
	{
		if(role != Qt::EditRole) return false;
		
		model->set_cell(index.row(), index.column(), to_boost_any<typename T::row_type>(value, index.column()));
		
		return true;
	}
	
	virtual Qt::ItemFlags flags(const QModelIndex &index) const
	{
		bool constness = is_const<typename T::row_type>(index.column());
	
		return Model::flags(index) | (constness ? Qt::NoItemFlags : Qt::ItemIsEditable);
	}
	
	virtual QVariant data(QModelIndex const& index, int role) const override
	{
		if(role != Qt::DisplayRole && role != Qt::EditRole) return QVariant();
		
		return to_qvariant<typename T::row_type>(model->get_cell(index.row(), index.column()), index.column());
	}
	
	template <bool b>
	typename std::enable_if<b, QVariant>::type get_key(int section) const
	{
		return QVariant(QString::fromStdString(model->key(section)));
	}
	
	template <bool b>
	typename std::enable_if<!b, QVariant>::type get_key(int section) const
	{
		return QVariant();
	}
	
	template <bool b>
	typename std::enable_if<b, QVariant>::type get_field_name(int section) const
	{
		return QVariant(QString::fromStdString(model->field_name(section)));
	}
	
	template <bool b>
	typename std::enable_if<!b, QVariant>::type get_field_name(int section) const
	{
		return QVariant();
	}
	
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override
	{	
		if(role != Qt::DisplayRole) {
			return QVariant();
		}
		
		if(orientation == Qt::Horizontal) {
			return get_field_name<T::has_header_h>(section);
		}
		
		if(orientation == Qt::Vertical) {
			return get_key<T::has_header_v>(section);
		}
		
		return QVariant();
	}
	
	//ObserverInterface
	
	virtual void cell_changed(int row, int column) override
	{
		emit this->dataChanged(this->createIndex(row, column), this->createIndex(row, column));
	}
	
	virtual void append_row_begin() override
	{
		this->beginInsertRows(QModelIndex(), this->rowCount(QModelIndex()), this->rowCount(QModelIndex()));
	}
	
	virtual void append_row_end() override
	{
		this->endInsertRows();
	}
};

template <typename T>
struct QtAdapter<T, QAbstractListModel> : public QtAdapterBase<T, QAbstractListModel>
{
	typedef QtWidget<QListView, T> widget;
	
	QtAdapter(std::shared_ptr<T> model)
	: QtAdapterBase<T, QAbstractListModel>(model)
	{}
};

template <typename T>
struct QtAdapter<T, QAbstractTableModel> : public QtAdapterBase<T, QAbstractTableModel>
{
	typedef QtWidget<QTableView, T> widget;
	
	QtAdapter(std::shared_ptr<T> model)
	: QtAdapterBase<T, QAbstractTableModel>(model)
	{}
	
};

template <typename Model>
struct AdapterType
{
	typedef QtAdapter<Model, typename QtModelType<typename Model::data_type>::type> type;
};

template <typename Model>
struct WidgetType {
	typedef typename AdapterType<Model>::type::widget type;
};

//Generate a default Table-based widget.
template <typename GuiType, typename Model, typename... Args>
typename std::enable_if<std::is_same<Gui::Table, GuiType>::value, std::shared_ptr<typename WidgetType<Model>::type>>::type
make_qt_widget(std::shared_ptr<Model> model, Args... args)
{
	auto widget_ptr = std::make_shared<typename WidgetType<Model>::type>(model, args...);
	return widget_ptr;
}

//Generate a default Form based widget.
template <typename GuiType, typename Model, typename... Args>
typename std::enable_if<std::is_same<Gui::Form, GuiType>::value, std::shared_ptr<Form<Model>>>::type
make_qt_widget(std::shared_ptr<Model> model, Args... args)
{
	auto widget_ptr = std::make_shared<Form<Model>>(model, args...);
	return widget_ptr;
}

//Generate a custom widget thats is neither a default Form or Table, for total specialisation of the function.
template <typename GuiType, typename Model, typename... Args>
typename std::enable_if<
	!std::is_same<Gui::Form, GuiType>::value &&
	!std::is_same<Gui::Table, GuiType>::value, std::shared_ptr<GuiType>
>::type
make_qt_widget(std::shared_ptr<Model> model, Args... args)
{
	auto widget_ptr = std::make_shared<GuiType>(model, args...);
	return widget_ptr;
}

