#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	MainWindow window;
	window.show();
	if(!window.init())
		return 0;
	else
		return application.exec();
}
