#pragma once

#include "fusion_static_dispatch.hpp"
#include "gui_item_delegate.hpp"
#include "boost_any_qvariant_convert.hpp"

#include <QMdiArea>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QValidator>

#include <memory>
#include <sstream>

template <bool is_int, bool is_float>
struct QValidator_impl
{
	static constexpr bool needed = false;
	typedef void type;
};

template <>
struct QValidator_impl<true, false>
{
	static constexpr bool needed = true;
	typedef QIntValidator type;
};

template <>
struct QValidator_impl<false, true>
{
	static constexpr bool needed = true;
	typedef QDoubleValidator type;
};

template <typename T>
struct QValidatorFor
{
	static constexpr bool needed = QValidator_impl<std::is_integral<T>::value, std::is_floating_point<T>::value>::needed;
	typedef typename QValidator_impl<std::is_integral<T>::value, std::is_floating_point<T>::value>::type type;
};

template <typename T>
struct CreateWidget
{
	typedef QLineEdit* return_type;
	
	template <bool allocate, typename V>
	static typename std::enable_if<allocate, void>::type
	set_validator_if_needed(QLineEdit* edit)
	{
		edit->setValidator(new V());
	}
	
	template <bool allocate, typename V>
	static typename std::enable_if<!allocate, void>::type
	set_validator_if_needed(QLineEdit*)
	{
		return;
	}
	
	template <int I>
	static return_type call()
	{
		QLineEdit* edit = new QLineEdit();
		
		typedef typename friendly_fusion::utils::DecayedTypeOfAtIndex<T, I>::type value_type;
		
		typedef QValidatorFor<value_type> QValiGen;
		
		set_validator_if_needed<QValiGen::needed, typename QValiGen::type>(edit);
		edit->setReadOnly(is_const<T>(I));
		return edit;
	}
};

template <typename T>
struct StringToBoostAny {
	
	typedef boost::any return_type;
	
	template <int I>
	static return_type call(std::string str)
	{
		typedef typename friendly_fusion::utils::DecayedTypeOfAtIndex<T, I>::type value_type;
		
		std::stringstream ss(str);
		
		value_type x;
		ss >> x;
		
		return boost::any(x);
	}
};

class FormUpdateHandler : public QObject
{
	
	Q_OBJECT
	
public:
	QLineEdit* edit;
	std::function<void(std::string)> cb;
	
	FormUpdateHandler(QLineEdit* edit, std::function<void(std::string)> cb)
	: edit(edit)
	, cb(cb)
	{
		connect(edit, SIGNAL(textEdited(QString const&)),
		        this, SLOT(edited(QString const&)));
	}
	
private slots:
	void edited(QString const& str)
	{
		cb(str.toStdString());
	}
	
};

template <typename T>
struct Form : public QFormLayout{
	
	std::shared_ptr<T> model;
	QMdiArea* area;
	std::vector<std::shared_ptr<FormUpdateHandler>> updatehandlers;
	std::map<int, std::map<int, QLineEdit*>> line_edits;
	GuiItemDelegate delegate;
	
	Form(std::shared_ptr<T> model)
	: model(model)
	, area(new QMdiArea())
	, updatehandlers()
	, line_edits()
	, delegate()
	{
		area->setLayout(this);
		setup_gui();
		fill_data();
	}
	
	void setup_gui()
	{
		for(int i = 0; i < model->row_count(); ++i) {
			for(int j = 0; j < model->column_count(); j++) {
				QLineEdit* edit = apply_functor_to_member<typename T::row_type, CreateWidget>(j);
				updatehandlers.push_back(std::make_shared<FormUpdateHandler>(edit, [i, j, this](std::string str){update_nth(i, j, str);}));
				line_edits[i][j] = edit;
				this->addRow(
					new QLabel(QString::fromStdString(model->field_name(j))), 
					edit
				);
			}
		}
	}
	
	void fill_data()
	{
		for(int i = 0; i < model->row_count(); ++i) {
			for(int j = 0; j < model->column_count(); j++) {
				line_edits[i][j]->setText(delegate.displayText(to_qvariant<typename T::row_type>(model->get_cell(i, j), j)));
			}
		}
	}
	
	void update_nth(int row, int column, std::string str)
	{
		model->set_cell(row, column, apply_functor_to_member<typename T::row_type, StringToBoostAny>(column, str));
	}
	
	QWidget* get_widget()
	{
		return area;
	}
};
