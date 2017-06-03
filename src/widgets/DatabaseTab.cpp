#include "src/widgets/DatabaseTab.hpp"
#include "ui_DatabaseTab.h"

#include <QSqlDatabase>
#include <QSqlQuery>

DatabaseTab::DatabaseTab(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::DatabaseTab)
{
	ui->setupUi(this);
}

void DatabaseTab::loadDatabases()
{
	if (databaseData.empty())
		return;

	QString driver = "QMYSQL";

	if (databaseData["type"] == "MySQL")
		driver = "QMYSQL";

	QSqlDatabase db = QSqlDatabase::addDatabase(driver, databaseData["name"]);
	db.setHostName(databaseData["hostname"]);
	db.setPort(databaseData["port"].toInt());
	db.setUserName(databaseData["username"]);
	db.setPassword(databaseData["password"]);

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

	QSqlDatabase::removeDatabase(databaseData["name"]);
}

DatabaseTab::~DatabaseTab()
{
	delete ui;
}
