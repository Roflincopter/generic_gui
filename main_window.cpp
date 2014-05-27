#include "main_window.hpp"

MainWindow::MainWindow()
: fake_argc(0)
, qapp(fake_argc, nullptr)
, w()
, area(new QMdiArea())
, layout(new QVBoxLayout())
{
	layout->setAlignment(Qt::AlignTop);
	area->setLayout(layout);
	
	w.setCentralWidget(area);
}

void MainWindow::add_widget(QWidget* widget)
{
	layout->addWidget(widget);
}

int MainWindow::show_and_run() {
	w.show();
	return qapp.exec();
}
