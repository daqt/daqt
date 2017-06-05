#include "src/widgets/ConnectionTab.hpp"
#include "ui_ConnectionTab.h"

#include <QSqlDatabase>
#include <QSqlQuery>

ConnectionTab::ConnectionTab(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::ConnectionTab)
{
	ui->setupUi(this);
}

void ConnectionTab::loadDatabases()
{
	if (connectionData.empty())
		return;

	QString driver = "QMYSQL";

	if (connectionData["type"] == "MySQL")
		driver = "QMYSQL";

	QSqlDatabase db = QSqlDatabase::addDatabase(driver, connectionData["name"]);
	db.setHostName(connectionData["hostname"]);
	db.setPort(connectionData["port"].toInt());
	db.setUserName(connectionData["username"]);
	db.setPassword(connectionData["password"]);

	if (db.open())
	{
		if (driver == "QMYSQL")
		{
			QSqlQuery query = db.exec("SHOW DATABASES;");

			while (query.next())
			{
				ui->listDatabases->addItem(query.value(0).toString());
			}
		}
	}

	db.close();
	db = QSqlDatabase();

	QSqlDatabase::removeDatabase(connectionData["name"]);
}

ConnectionTab::~ConnectionTab()
{
	delete ui;
}
