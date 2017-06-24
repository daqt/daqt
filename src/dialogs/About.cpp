#include "src/dialogs/About.hpp"
#include "ui_About.h"

About::About(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::About)
{
	ui->setupUi(this);
}

About::~About()
{
	delete ui;
}
