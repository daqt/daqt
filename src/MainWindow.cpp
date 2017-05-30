#include "src/MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QSignalMapper>

#include "src/dialogs/NewConnection.hpp"

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	QSignalMapper* mapper = new QSignalMapper(this);

	connect(ui->buttonNewConnection, SIGNAL(pressed()), mapper, SLOT(map()));
	mapper->setMapping(ui->buttonNewConnection, DIALOG_NEWCONNECTION);

	connect(ui->actionNewConnection, SIGNAL(triggered()), mapper, SLOT(map()));
	mapper->setMapping(ui->actionNewConnection, DIALOG_NEWCONNECTION);

	connect(mapper, SIGNAL(mapped(int)), this, SLOT(showDialog(int)));
}

void MainWindow::showDialog(int dialog)
{
	QDialog* qDialog;

	switch (dialog)
	{
	case DIALOG_NEWCONNECTION:
		qDialog = new NewConnection(this);
		break;
	}

	qDialog->setModal(true);
	qDialog->show();
}

MainWindow::~MainWindow()
{
	delete ui;
}
