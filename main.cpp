
#include "fusion_model.hpp"
#include "qt_adapter.hpp"
#include "gui_item_delegate.hpp"
#include "meta_types.hpp"

#include <boost/fusion/adapted.hpp>

#include <iostream>

#include <QMainWindow>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QApplication>
#include <QMdiArea>

struct Data {
	std::string name;
	uint32_t number;
	float ratio;
	bool boolean;
};

BOOST_FUSION_ADAPT_STRUCT(
	Data,
	(std::string, name)
	(uint32_t, number)
	(float, ratio)
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

int main()
{
	Data d1{"Pietje", 2, 3.333f, true};
	Data d2{"Jantje", 3, 1.5f, false};
	Data d3{"Sjaakje", 1, 0.1337f, false};
	
	DataModel model;
	
	model.add_data(d1);
	model.add_data(d2);
	model.add_data(d3);
	
	auto adapter = make_qt_adapter(model);
	
	DataMapping mapping;
	
	mapping.add_data("nummer1", d1);
	mapping.add_data("nummer2", d2);
	mapping.add_data("nummer3", d3);
	
	auto&& adapter2 = make_qt_adapter(mapping);
	
	int argc = 0;
	QApplication qapp(argc, nullptr);
	
	QMainWindow w;
	
	widget_type<decltype(adapter)>::type widget;
	widget.setFixedSize(640, 480);
	widget.setModel(adapter.get());
	//widget.show();
	
	widget_type<decltype(adapter)>::type widget2;
	widget2.setFixedSize(640, 480);
	widget2.verticalHeader()->setEnabled(true);
	widget2.setModel(adapter2.get());
	//widget2.show();
	
	QMdiArea area;
	
	QVBoxLayout layout;

	layout.addWidget(&widget);
	layout.addWidget(&widget2);
	
	area.setLayout(&layout);
	
	w.setCentralWidget(&area);
	w.show();
	
	return qapp.exec();
}
