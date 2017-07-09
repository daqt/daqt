#include "Query.hpp"

#include <QDir>
#include <QSqlQuery>
#include <QVariant>

QStringList Query::listDatabases(QSqlDatabase* db, QString driver)
{
	QStringList result;
	QSqlQuery query(*db);

	if (driver == "QSQLITE")
	{
		result.append((*db).databaseName().split(QDir::separator()).last());

		return result;
	}
	else if (driver == "QMYSQL")
	{
		query.prepare("SELECT `SCHEMA_NAME` FROM `information_schema`.`SCHEMATA`");
	}

	query.exec();

	while (query.next())
	{
		result.append(query.value(0).toString());
	}

	return result;
}

QStringList Query::listTables(QSqlDatabase* db, QString driver, QString database)
{
	QStringList result;
	QSqlQuery query(*db);

	if (driver == "QMYSQL")
	{
		query.prepare("SELECT `TABLE_NAME` FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA`='" + database + "';");
	}
	else if (driver == "QSQLITE")
	{
		query.prepare("SELECT `name` FROM `sqlite_master` WHERE `type`='table'");
	}

	query.exec();

	while (query.next())
	{
		result.append(query.value(0).toString());
	}

	return result;
}
