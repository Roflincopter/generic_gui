#pragma once

#include "fusion_static_dispatch.hpp"
#include "gui_item_delegate.hpp"
#include "boost_any_qvariant_convert.hpp"
#include "fusion_model.hpp"

#include <QMdiArea>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QValidator>
#include <QPushButton>

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
		ss >> std::boolalpha >> x;
		
		return boost::any(x);
	}
};

template <typename T>
struct BoostAnyToString {
	
	typedef std::string return_type;
	
	template <int I>
	static return_type call(boost::any any)
	{
		typedef typename friendly_fusion::utils::DecayedTypeOfAtIndex<T, I>::type value_type;
		
		std::stringstream ss;
		ss << std::boolalpha << boost::any_cast<value_type>(any);
		
		return ss.str();
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
		connect(
			edit, SIGNAL(textEdited(QString const&)),
			this, SLOT(edited(QString const&))
		);
	}
	
private slots:
	void edited(QString const& str)
	{
		cb(str.toStdString());
	}
	
};

//This base is required because an QObject may not be templated and Form is a class template.
//Slots can be virtual however.
class QtFormBase : public QObject
{

	Q_OBJECT

protected slots:
	virtual void prev_clicked() = 0;
	virtual void jump() = 0;
	virtual void next_clicked() = 0;
};

template <typename T>
struct Form : public QtFormBase, public FusionModelObserver {
	
	std::shared_ptr<T> model;
	QMdiArea* area;
	QVBoxLayout* layout;
	QFormLayout* form_layout;
	QHBoxLayout* button_layout;
	QPushButton* prev;
	QLineEdit* jump_edit;
	QPushButton* next;
	std::vector<std::shared_ptr<FormUpdateHandler>> updatehandlers;
	std::map<int, QLineEdit*> line_edits;
	
	int current_index;
	
	Form(std::shared_ptr<T> model)
	: model(model)
	, area(new QMdiArea())
	, layout(new QVBoxLayout())
	, form_layout(new QFormLayout())
	, button_layout(new QHBoxLayout())
	, prev(new QPushButton("&Previous"))
	, jump_edit(new QLineEdit())
	, next(new QPushButton("&Next"))
	, updatehandlers()
	, line_edits()
	, current_index(0)
	{
		layout->setAlignment(Qt::AlignTop);
		
		prev->setMaximumWidth(150);
		
		jump_edit->setValidator(new QIntValidator());
		jump_edit->setMaximumWidth(100);
		
		next->setMaximumWidth(150);
		
		layout->addLayout(form_layout);
		layout->addLayout(button_layout);
		
		button_layout->addWidget(prev);
		button_layout->addWidget(jump_edit);
		button_layout->addWidget(next);
		button_layout->setAlignment(Qt::AlignHCenter);
		
		area->setLayout(layout);
		
		setup_gui();
		fill_data();
		
		area->updateGeometry();
		
		set_button_state();
		connect(
			prev, SIGNAL(clicked()),
			this, SLOT(prev_clicked())
		);
		
		connect(
			jump_edit, SIGNAL(editingFinished()),
			this, SLOT(jump())
		);
		
		connect(
			next, SIGNAL(clicked()),
			this, SLOT(next_clicked())
		);
		
		set_current_index(0);
	}
	
	void setup_gui()
	{
		for(int j = 0; j < model->column_count(); j++) {
			QLineEdit* edit = apply_functor_to_member<typename T::row_type, CreateWidget>(j);
			updatehandlers.push_back(std::make_shared<FormUpdateHandler>(edit, [j, this](std::string str){update_nth_in_model(j, str);}));
			line_edits[j] = edit;
			form_layout->addRow(
				new QLabel(QString::fromStdString(model->field_name(j))), 
				edit
			);
		}
	}
	
	void update_pos(int i, int j)
	{
		line_edits[j]->setText(QString::fromStdString(apply_functor_to_member<typename T::row_type, BoostAnyToString>(j, model->get_cell(i, j))));
	}
	
	void fill_data()
	{
		for(int j = 0; j < model->column_count(); j++) {
			update_pos(current_index, j);
		}
	}
	
	void set_button_state()
	{
		prev->setEnabled(current_index != 0);
		next->setEnabled(current_index != model->row_count() - 1);
	}
	
	void update_nth_in_model(int entry, std::string str)
	{
		model->set_cell(current_index, entry, apply_functor_to_member<typename T::row_type, StringToBoostAny>(entry, str));
	}
	
	void set_current_index(int i)
	{
		current_index = i;
		fill_data();
		set_button_state();
		jump_edit->setText(QString::number(i + 1));
	}
	
	int clamp_index(int i)
	{
		if(i < 0) {
			i = 0;
		}
		
		if(i > model->row_count() - 1) {
			i = model->row_count() - 1;
		}
		
		return i;
	}
	
	//Observer interface
	virtual void cell_changed(int form_id, int entry) override final
	{
		update_pos(form_id, entry);
	}
	
	//Gui interface.
	virtual void prev_clicked() override final
	{
		int i = clamp_index(current_index - 1);
		set_current_index(i);
	}
	
	virtual void jump() override final
	{
		int jump_to = jump_edit->text().toInt() - 1;
		int i = clamp_index(jump_to);
		
		set_current_index(i);
	}
	
	virtual void next_clicked() override final
	{
		int i = clamp_index(current_index + 1);
		set_current_index(i);
	}
	
	QWidget* get_widget()
	{
		return area;
	}
};

template <typename Model>
std::shared_ptr<Form<Model>> make_form(std::shared_ptr<Model> model)
{
	auto ret = std::make_shared<Form<Model>>(model);
	model->add_observer(ret);
	return ret;
}
