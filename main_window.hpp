#pragma once

#include <QMainWindow>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QApplication>
#include <QMdiArea>

struct MainWindow {
	
	int fake_argc;
	QApplication qapp;
	QMainWindow w;
	QMdiArea* area = new QMdiArea();
	QVBoxLayout* layout = new QVBoxLayout();
	
	MainWindow();
	
	void add_widget(QWidget* widget);

	int show_and_run();
};