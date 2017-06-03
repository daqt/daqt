#include "src/MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QMap>
#include <QSignalMapper>
#include <QStandardItemModel>

#include "src/dialogs/NewConnection.hpp"
#include "src/widgets/DatabaseTab.hpp"
#include "src/SavedDatabases.hpp"

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

	loadDatabases(0);

	connect(ui->tableDatabases, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openDatabase(QModelIndex)));

	ui->tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->resize(0, 0);

	connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
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

void MainWindow::openDatabase(QModelIndex index)
{
	QString name = index.sibling(index.row(), 0).data().toString();

	DatabaseTab* newTab = new DatabaseTab(ui->tabWidget);

	ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(newTab, name));

	newTab->setDatabaseData(SavedDatabases::getDatabase(name));
	newTab->loadDatabases();
}

void MainWindow::closeTab(int tab)
{
	if (tab > 0)
		ui->tabWidget->removeTab(tab);
}

void MainWindow::loadDatabases(int)
{
	QMap<QString, QMap<QString, QString>> databases = SavedDatabases::getDatabases();

	QStandardItemModel* model = new QStandardItemModel(this);

	QStringList header;
	header.append("Name");
	header.append("Type");
	header.append("Hostname");
	header.append("Username");

	model->setHorizontalHeaderLabels(header);

	for (QMap<QString, QMap<QString, QString>>::iterator it = databases.begin(); it != databases.end(); it++)
	{
		QList<QStandardItem*> data;
		data.append(new QStandardItem(it.key()));
		data.append(new QStandardItem(it.value()["type"]));
		data.append(new QStandardItem(it.value()["hostname"] + ":" + it.value()["port"]));
		data.append(new QStandardItem(it.value()["username"]));

		model->appendRow(data);
	}

	ui->tableDatabases->setModel(model);
	ui->tableDatabases->resizeRowsToContents();
	ui->tableDatabases->resizeColumnsToContents();
	ui->tableDatabases->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
	delete ui;
}
