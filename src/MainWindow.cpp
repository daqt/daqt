#include "src/MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QMap>
#include <QSignalMapper>
#include <QStandardItemModel>

#include "src/dialogs/NewConnection.hpp"
#include "src/widgets/ConnectionTab.hpp"
#include "src/SavedConnections.hpp"

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

	connect(ui->tableConnections, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openConnection(QModelIndex)));

	connect(ui->tableConnections, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableContext(QPoint)));

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
	case DIALOG_EDITCONNECTION:
		if (!ui->tableConnections->selectionModel()->hasSelection())
			return;

		qDialog = new NewConnection(this);
		qDialog->setWindowTitle("Edit connection");

		QString selected = ui->tableConnections->selectionModel()->selectedRows()[0].data().toString();
		((NewConnection*)qDialog)->setValues(SavedConnections::getConnection(selected));
		break;
	}

	qDialog->setModal(true);
	qDialog->show();
}

void MainWindow::openConnection(QModelIndex index)
{
	QString name = index.sibling(index.row(), 0).data().toString();

	ConnectionTab* newTab = new ConnectionTab(ui->tabWidget);

	ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(newTab, name));

	newTab->setConnectionData(SavedConnections::getConnection(name));
	newTab->loadDatabases();
}

void MainWindow::closeTab(int tab)
{
	if (tab > 0)
		ui->tabWidget->removeTab(tab);
}

void MainWindow::loadDatabases(int)
{
	QMap<QString, QMap<QString, QString>> databases = SavedConnections::getConnections();

	QStandardItemModel* model = new QStandardItemModel(this);

	QStringList header;
	header.append("Name");
	header.append("Driver");
	header.append("Hostname");
	header.append("Username");

	model->setHorizontalHeaderLabels(header);

	for (QMap<QString, QMap<QString, QString>>::iterator it = databases.begin(); it != databases.end(); it++)
	{
		QList<QStandardItem*> data;
		data.append(new QStandardItem(it.key()));
		data.append(new QStandardItem(it.value()["driver"]));
		data.append(new QStandardItem(it.value()["hostname"] + ":" + it.value()["port"]));
		data.append(new QStandardItem(it.value()["username"]));

		model->appendRow(data);
	}

	ui->tableConnections->setModel(model);
	ui->tableConnections->resizeColumnsToContents();
	ui->tableConnections->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::tableContext(QPoint point)
{
	QPoint pos = ui->tableConnections->mapToGlobal(point);
	QMenu tableMenu;

	tableMenu.addAction("Edit");
	tableMenu.addAction("Remove");

	QAction* action = tableMenu.exec(pos);
	if (action)
	{
		if (action->text() == "Edit")
			showDialog(DIALOG_EDITCONNECTION);

		if (action->text() == "Remove")
		{
			if (!ui->tableConnections->selectionModel()->hasSelection())
				return;

			QString selected = ui->tableConnections->selectionModel()->selectedRows()[0].data().toString();
			SavedConnections::removeConnection(selected);

			loadDatabases(0);
		}
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}
