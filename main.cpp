
#include "main_window.hpp"
#include "fusion_model.hpp"
#include "qt_adapter.hpp"
#include "meta_types.hpp"
#include "fusion_outputter.hpp"

#include <boost/fusion/adapted.hpp>

#include <iostream>

struct Data {
	std::string name;
	uint32_t number;
	double ratio1;
	float ratio2;
	bool boolean;
};

BOOST_FUSION_ADAPT_STRUCT(
	Data,
	(std::string, name)
	(uint32_t, number)
	(double, ratio1)
	(float,ratio2)
	(bool, boolean)
)

struct DataModel : public fusion_model<std::vector<Data>> {
	
	void add_data(Data d) {
		data.push_back(d);
	}
};

struct DataMapping :public fusion_model<std::map<std::string, Data>> {
	void add_data(std::string key, Data value)
	{
		data.emplace(key, value);
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
	Data d1{"Pietje", 2, 3.333, 0.333, true};
	Data d2{"Jantje", 3, 1.5, 0.5, false};
	Data d3{"Sjaakje", 1, 0.1337, 0.0337, false};
	
	auto model = std::make_shared<DataModel>();
	
	model->add_data(d1);
	model->add_data(d2);
	model->add_data(d3);
	
	auto mapping = std::make_shared<DataMapping>();
	
	mapping->add_data("nummer1", d1);
	mapping->add_data("nummer2", d2);
	mapping->add_data("nummer3", d3);

	MainWindow w;
	
	auto widget1 = std::make_shared<CustomDataModelWidget>(model);
	auto widget2 = make_qt_widget(model);
	auto widget3 = make_qt_widget(mapping);
	
	w.add_widget(widget1.get());
	w.add_widget(widget2.get());
	w.add_widget(widget3.get());
	
	int ret = w.show_and_run();
	
	std::cout << "model: " << std::endl << model->data << std::endl;
	std::cout << "mapping: " << std::endl << mapping->data << std::endl;
	
	return ret;
}
