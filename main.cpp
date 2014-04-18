
#include "fusion_model.hpp"
#include "qt_adapter.hpp"
#include "gui_item_delegate.hpp"

#include <boost/fusion/adapted.hpp>

#include <iostream>

#include <QMainWindow>
#include <QApplication>

struct Data {
	std::string name;
	uint32_t number;
	float ratio;
	bool lolwut;
};

BOOST_FUSION_ADAPT_STRUCT(
	Data,
	(std::string, name)
	(uint32_t, number)
	(float, ratio)
	(bool, lolwut)
)

struct DataModel : public fusion_model<std::vector<Data>> {
	
	void add_data(Data d) {
		data.push_back(d);
	}
};

int main()
{
	DataModel model;
	
	model.add_data({"Pietje", 2, 3.333f, true});
	model.add_data({"Jantje", 3, 1.5f, false});
	model.add_data({"Sjaakje", 1, 0.1337f, false});
	
	auto adapter = make_qt_adapter(model);
	
	int argc = 0;
	QApplication qapp(argc, nullptr);
	
	QMainWindow w;

	decltype(adapter)::element_type::view view;
	
	view.setModel(adapter.get());
	
	GuiItemDelegate delegate;
	view.setItemDelegate(&delegate);
	
	view.show();
	
	qapp.exec();
	
	for(int column = 0; column < model.column_count(); ++column)
	{
		std::cout << model.horizontal_header_data(column) << "\t";
	}
	std::cout << std::endl;
	
	for(int row = 0; row < model.row_count(); ++row)
	{
		for(int column = 0; column < model.column_count(); ++column) {
			if(column == 0) {
				std::cout << boost::any_cast<std::string>(model.get_cell(row, column)) << "\t";
			} else {
				std::cout << boost::any_cast<uint32_t>(model.get_cell(row, column)) << "\t";
			}
		}
		std::cout << std::endl;
	}
	
	return 0;
}
