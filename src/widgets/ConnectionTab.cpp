#include "src/widgets/ConnectionTab.hpp"
#include "ui_ConnectionTab.h"

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTableWidgetItem>

ConnectionTab::ConnectionTab(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::ConnectionTab)
{
	ui->setupUi(this);

	connect(ui->listDatabases, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(loadTables(QModelIndex)));
	connect(ui->listTables, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openTable(QModelIndex)));
	connect(ui->tableValues, SIGNAL(cellChanged(int, int)), this, SLOT(changeValue(int, int)));
}

void ConnectionTab::setConnectionData(QMap<QString, QString> data)
{
	connectionData = data;

	if (connectionData.empty())
		return;

	QString driver = "QMYSQL";

	if (connectionData["driver"] == "MySQL")
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
		if (connectionData["driver"] == "MySQL")
		{
			QSqlQuery query(db);
			query.prepare("SELECT `SCHEMA_NAME` FROM `information_schema`.`SCHEMATA`");
			query.exec();

			while (query.next())
			{
				ui->listDatabases->addItem(query.value(0).toString());
			}
		}

		db.close();
	}
}

void ConnectionTab::loadTables(QModelIndex index)
{
	if (db.open())
	{
		ui->listTables->clear();

		databaseName = index.data().toString();

		if (connectionData["driver"] == "MySQL")
		{
			db.setDatabaseName(databaseName);

			QSqlQuery query(db);
			query.prepare("SELECT `TABLE_NAME` FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA`='" + databaseName + "';");
			query.exec();

			while (query.next())
			{
				ui->listTables->addItem(query.value(0).toString());
			}
		}

		db.close();
	}
}

void ConnectionTab::openTable(QModelIndex index)
{
	ui->tableValues->blockSignals(true);

	ui->tableValues->setRowCount(0);
	ui->tableValues->clearContents();

	if (db.open())
	{
		tableName = index.data().toString();

		if (connectionData["driver"] == "MySQL")
		{
			QSqlQuery query(db);

			query.prepare("SELECT `COLUMN_NAME`,`DATA_TYPE` FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA`='" + db.databaseName() + "' AND `TABLE_NAME`='" + tableName + "';");
			query.exec();

			QStringList header;

			while (query.next())
			{
				header.append(query.value(0).toString() + " (" + query.value(1).toString() + ")");
			}

			ui->tableValues->setColumnCount(header.length());
			ui->tableValues->setHorizontalHeaderLabels(header);

			query.prepare("SELECT `ORDINAL_POSITION` FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA`='" + db.databaseName() + "' AND `TABLE_NAME`='" + tableName + "' AND `CONSTRAINT_NAME`='PRIMARY';");
			query.exec();

			while (query.next())
			{
				QTableWidgetItem* primary = ui->tableValues->horizontalHeaderItem(query.value(0).toInt() - 1);
				primary->setTextColor(QColor().fromRgb(0xDD, 0xDD, 0x00));
				ui->tableValues->setHorizontalHeaderItem(query.value(0).toInt() - 1, primary);
			}

			query.prepare("SELECT * FROM `" + tableName + "`");
			query.exec();

			while (query.next())
			{
				ui->tableValues->insertRow(ui->tableValues->rowCount());

				for (int i = 0; i < query.record().count(); i++)
				{
					ui->tableValues->setItem(ui->tableValues->rowCount() - 1, i, new QTableWidgetItem(query.value(i).toString()));
				}
			}

			ui->tableValues->resizeColumnsToContents();
		}

		db.close();
	}

	ui->tableValues->blockSignals(false);
}

void ConnectionTab::changeValue(int row, int column)
{
	if (db.open())
	{
		QString primary = "id";
		int primaryIndex = 0;

		if (connectionData["driver"] == "MySQL")
		{
			QSqlQuery query(db);

			query.prepare("SELECT `COLUMN_NAME`,`ORDINAL_POSITION` FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA`='" + db.databaseName() + "' AND `TABLE_NAME`='" + tableName + "' AND `CONSTRAINT_NAME`='PRIMARY';");
			query.exec();
			query.next();

			primary = query.value(0).toString();
			primaryIndex = query.value(1).toInt() - 1;

			QString columnName = ui->tableValues->horizontalHeaderItem(column)->text().split(' ')[0];
			QString primaryVal = ui->tableValues->item(row, primaryIndex)->text();

			query.prepare("UPDATE `" + tableName + "` SET `" + columnName + "`=? WHERE `" + primary + "`='" + primaryVal + "'");
			query.addBindValue(ui->tableValues->item(row, column)->text());
			query.exec();
		}

		db.close();
	}
}

ConnectionTab::~ConnectionTab()
{
	db = QSqlDatabase();
	QSqlDatabase::removeDatabase(connectionData["name"]);

	delete ui;
}
