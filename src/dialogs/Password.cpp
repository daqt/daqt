#include "src/dialogs/Password.hpp"
#include "ui_Password.h"

Password::Password(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::Password)
{
	ui->setupUi(this);

	connect(ui->buttonSend, SIGNAL(pressed()), this, SLOT(send()));
}

void Password::send()
{
	emit sendPassword(ui->editPassword->text(), ui->checkPassword->isChecked());

	this->accept();
}

Password::~Password()
{
	delete ui;
}
