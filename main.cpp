
#include "main_window.hpp"
#include "fusion_model.hpp"
#include "qt_adapter.hpp"
#include "meta_types.hpp"
#include "fusion_outputter.hpp"
#include "form.hpp"

#include <boost/fusion/adapted.hpp>

#include <iostream>

struct Data {
	const std::string name;
	std::string gender;
	uint32_t number;
	const double ratio1;
	double ratio2;
	bool boolean;
};

BOOST_FUSION_ADAPT_STRUCT(
	Data,
	(const std::string, name)
	(std::string, gender)
	(uint32_t, number)
	(const double, ratio1)
	(double,ratio2)
	(bool, boolean)
)

struct DataModel : public FusionModel<std::vector<Data>> {
	
	std::vector<Data> model;
	
	DataModel()
	: FusionModel(model)
	{}
	
	void add_data(Data d) {
		model.push_back(d);
	}
};

struct DataMapping : public FusionModel<std::map<std::string, Data>> {
	
	std::map<std::string, Data> model;
	
	DataMapping()
	: FusionModel(model)
	{}
	
	void add_data(std::string key, Data value)
	{
		call_on_observers(&FusionModelObserver::append_row_begin);
		data.emplace(key, value);
		call_on_observers(&FusionModelObserver::append_row_end);
	}
};

struct CustomDataModelWidget : public WidgetType<DataModel>::type
{
	CustomDataModelWidget(std::shared_ptr<DataModel> model)
	: WidgetType<DataModel>::type(model)
	{}
};

int main()
{
	Data d1{"Jan", "Male", 1, 3.333, 0.333, true};
	Data d2{"Piet", "Male",2, 1.5, 0.5, false};
	Data d3{"Klaas", "Confused", 3, 0.1337, 0.0337, false};
	
	auto model = std::make_shared<DataModel>();
	
	model->add_data(d1);
	model->add_data(d2);
	model->add_data(d3);
	
	auto mapping = std::make_shared<DataMapping>();
	
	mapping->add_data("nummer1", d1);
	mapping->add_data("nummer2", d2);
	mapping->add_data("nummer3", d3);

	MainWindow w;
	
	auto widget1 = make_qt_widget<CustomDataModelWidget>(model);
	auto widget2 = make_qt_widget<Gui::Table>(model);
	auto widget3 = make_qt_widget<Gui::Table>(mapping);
	auto widget4 = make_qt_widget<Gui::Form>(model);
	
	w.add_widget(widget1.get());
	w.add_widget(widget2.get());
	w.add_widget(widget3.get());
	w.add_widget(widget4.get());;
	
	int ret = w.show_and_run();
	
	std::cout << "model: " << std::endl << model->data << std::endl;
	std::cout << "mapping: " << std::endl << mapping->data << std::endl;
	
	return ret;
}
