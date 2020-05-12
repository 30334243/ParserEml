#pragma once
#include <Header.h>
#include "ParserEml.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ParserEml w;
	w.show();
	return a.exec();
}
