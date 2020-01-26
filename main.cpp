#include "las2xlsconverter.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	LAS2XLSConverter w;
	w.show();
	return a.exec();
}
