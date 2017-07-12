#include "src/MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QDir>
#include <QList>
#include <QSignalMapper>
#include <QStandardItemModel>

#include "src/dialogs/NewConnection.hpp"
#include "src/dialogs/About.hpp"
#include "src/types/Connection.hpp"
#include "src/Utils.hpp"
#include "src/widgets/ConnectionTab.hpp"

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
	connect(ui->actionAbout_Daqt, SIGNAL(triggered()), mapper, SLOT(map()));
	mapper->setMapping(ui->actionAbout_Daqt, DIALOG_ABOUT);
	connect(mapper, SIGNAL(mapped(int)), this, SLOT(showDialog(int)));

	loadConnections(0);

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
	case DIALOG_ABOUT:
		qDialog = new About(this);

		qDialog->setFixedSize(480, 320);
		break;
	case DIALOG_EDITCONNECTION:
		if (!ui->tableConnections->selectionModel()->hasSelection())
			return;

		qDialog = new NewConnection(this);
		QString selected = ui->tableConnections->selectionModel()->selectedRows()[0].data().toString();
		Connection* connection = new Connection(selected);
		connection->load();

		((NewConnection*)qDialog)->setValues(connection);

		break;
	}

	qDialog->setModal(true);
	qDialog->exec();
}

void MainWindow::openConnection(QModelIndex index)
{
	QString name = index.sibling(index.row(), 0).data().toString();

	ConnectionTab* newTab = new ConnectionTab(ui->tabWidget);

	connect(newTab, SIGNAL(finished()), this, SLOT(closeCurrentTab()));

	ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(newTab, name));

	Connection* connection = new Connection(name);
	connection->load();

	newTab->setConnectionData(connection);
	newTab->open(1);
}

void MainWindow::closeTab(int tab)
{
	if (tab > 0)
	{
		ui->tabWidget->removeTab(tab);
	}
}

void MainWindow::loadConnections(int)
{
	QList<Connection*> connections;

	QStringList list = QDir(Utils::getConfigDirectory().absolutePath() + QDir::separator() + "connections").entryList(QDir::NoDotAndDotDot | QDir::AllDirs);

	for (QStringList::iterator it = list.begin(); it != list.end(); it++)
	{
		QString dir = it.i->t();

		if (!QDir().exists(Utils::getConfigDirectory().absolutePath() + QDir::separator() + "connections" + QDir::separator() + dir))
		{
			continue;
		}

		Connection* con = new Connection(dir);
		con->load();

		connections.push_back(con);
	}

	QStandardItemModel* model = new QStandardItemModel(this);

	QStringList header;
	header.append("Name");
	header.append("Driver");
	header.append("Host");
	header.append("Username");

	model->setHorizontalHeaderLabels(header);

	for (int c = 0; c < connections.length(); c++)
	{
		Connection* connection = connections[c];

		QList<QStandardItem*> data;
		data.append(new QStandardItem(connection->getName()));

		if (connection->getDriver() == "QMYSQL" || connection->getDriver() == "QPSQL")
		{
			if (connection->getDriver() == "QMYSQL")
			{
				data.append(new QStandardItem("MySQL"));
			}
			else if (connection->getDriver() == "QPSQL")
			{
				data.append(new QStandardItem("PostgreSQL"));
			}

			data.append(new QStandardItem(connection->getHost().host() + ":" + QString::number(connection->getHost().port())));
			data.append(new QStandardItem(connection->getUsername()));
		}
		else if (connection->getDriver() == "QSQLITE")
		{
			data.append(new QStandardItem("SQLite"));

			data.append(new QStandardItem(connection->getPath()));
		}

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
		{
			showDialog(DIALOG_EDITCONNECTION);
		}

		if (action->text() == "Remove")
		{
			if (!ui->tableConnections->selectionModel()->hasSelection())
			{
				return;
			}

			QString selected = ui->tableConnections->selectionModel()->selectedRows()[0].data().toString();
			QDir dir(Utils::getConfigDirectory().absolutePath() + QDir::separator() + "connections" + QDir::separator() + selected);

			dir.removeRecursively();

			loadConnections(0);
		}
	}
}

void MainWindow::closeCurrentTab()
{
	closeTab(ui->tabWidget->currentIndex());
}

MainWindow::~MainWindow()
{
	delete ui;
}
