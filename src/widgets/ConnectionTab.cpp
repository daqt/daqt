#include "src/widgets/ConnectionTab.hpp"
#include "ui_ConnectionTab.h"

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>

ConnectionTab::ConnectionTab(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::ConnectionTab)
{
	ui->setupUi(this);

	connect(ui->listDatabases, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(loadTables(QModelIndex)));
	connect(ui->listTables, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openTable(QModelIndex)));
}

void ConnectionTab::setConnectionData(QMap<QString, QString> data)
{
	connectionData = data;

	if (connectionData.empty())
		return;

	QString driver = "QMYSQL";

	if (connectionData["type"] == "MySQL")
		driver = "QMYSQL";

	db = QSqlDatabase::addDatabase(driver, connectionData["name"]);
	db.setHostName(connectionData["hostname"]);
	db.setPort(connectionData["port"].toInt());
	db.setUserName(connectionData["username"]);
	db.setPassword(connectionData["password"]);
}

void ConnectionTab::loadDatabases()
{
	if (db.open())
	{
		if (connectionData["type"] == "MySQL")
		{
			QSqlQuery query(db);
			query.prepare("SHOW DATABASES;");
			query.exec();

			while (query.next())
			{
				ui->listDatabases->addItem(query.value(0).toString());
			}
		}
	}

	db.close();
}

void ConnectionTab::loadTables(QModelIndex index)
{
	if (db.open())
	{
		ui->listTables->clear();

		QString databaseName = index.data().toString();

		if (connectionData["type"] == "MySQL")
		{
			db.setDatabaseName(databaseName);

			QSqlQuery query(db);
			query.prepare("USE " + databaseName + ";");
			query.exec();

			query.prepare("SHOW TABLES;");
			query.exec();

			while (query.next())
			{
				ui->listTables->addItem(query.value(0).toString());
			}
		}
	}

	db.close();
}

void ConnectionTab::openTable(QModelIndex index)
{
	if (db.open())
	{
		QString tableName = index.data().toString();

		if (connectionData["type"] == "MySQL")
		{
			QSqlQuery query(db);
			query.prepare("SELECT * FROM " + tableName + ";");
			query.exec();

			QStandardItemModel* model = new QStandardItemModel(this);

			QStringList header;
			QSqlRecord record = query.record();

			for (int i = 0; i < record.count(); i++)
			{
				header.append(record.fieldName(i));
			}

			model->setHorizontalHeaderLabels(header);

			while (query.next())
			{
				QList<QStandardItem*> data;

				for (int i = 0; i < record.count(); i++)
				{
					data.append(new QStandardItem(query.value(i).toString()));
				}

				model->appendRow(data);
			}

			ui->tableValues->setModel(model);
			ui->tableValues->resizeColumnsToContents();
		}
	}

	db.close();
}

ConnectionTab::~ConnectionTab()
{
	db = QSqlDatabase();
	QSqlDatabase::removeDatabase(connectionData["name"]);

	delete ui;
}
