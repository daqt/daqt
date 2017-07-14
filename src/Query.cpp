#include "Query.hpp"

#include <QDir>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

#include "src/Utils.hpp"

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
		query.prepare("SELECT `SCHEMA_NAME` FROM `information_schema`.`SCHEMATA`;");
	}
	else if (driver == "QPSQL")
	{
		query.prepare("SELECT \"datname\" FROM \"pg_database\" WHERE \"datistemplate\"=false;");
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
		query.prepare("SELECT `name` FROM `sqlite_master` WHERE `type`='table';");
	}
	else if (driver == "QPSQL")
	{
		query.prepare("SELECT \"table_schema\",\"table_name\" FROM \"information_schema\".\"tables\" WHERE \"table_catalog\"='" + database + "' AND \"table_type\"='BASE TABLE';");
	}

	query.exec();

	while (query.next())
	{
		if (driver == "QPSQL")
		{
			result.append(query.value(0).toString() + "." + query.value(1).toString());
		}
		else
		{
			result.append(query.value(0).toString());
		}
	}

	return result;
}

QStringList Query::getHeader(QSqlDatabase* db, QString driver, QString database, QString table)
{
	QStringList result;
	QSqlQuery query(*db);

	if (driver == "QMYSQL")
	{
		query.prepare("SELECT `COLUMN_NAME`,`DATA_TYPE` FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA`='" + database + "' AND `TABLE_NAME`='" + table + "';");
		query.exec();

		while (query.next())
		{
			result.append(query.value(0).toString() + " (" + query.value(1).toString() + ")");
		}

		query.prepare("SELECT `ORDINAL_POSITION` FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA`='" + database + "' AND `TABLE_NAME`='" + table + "' AND `CONSTRAINT_NAME`='PRIMARY';");
		query.exec();

		while (query.next())
		{
			QString primary = result.at(query.value(0).toInt() - 1);

			primary = primary.remove(primary.length() - 1, 1);
			primary += " P)";

			result.replace(query.value(0).toInt() - 1, primary);
		}
	}
	else if (driver == "QSQLITE")
	{
		query.prepare("SELECT `sql` FROM `sqlite_master` WHERE `name`='" + table + "' AND `type`='table';");
		query.exec();

		query.next();

		QString sql = query.value(0).toString();
		sql = sql.mid(sql.indexOf('(') + 1);
		sql = sql.remove(sql.length() - 1, 1);

		QStringList parts = sql.split(',');

		for (int p = 0; p < parts.length(); p++)
		{
			QString part = parts[p];
			part = part.trimmed();

			if (part.startsWith("PRIMARY"))
			{
				QString primaryName = part.mid(part.indexOf('`') + 1);
				primaryName = primaryName.mid(0, primaryName.indexOf('`'));

				int i = Utils::findInList<QString>(result, primaryName);

				QString primary = result.at(i);

				primary = primary.remove(primary.length() - 1, 1);
				primary += " P)";

				result.replace(i, primary);

				continue;
			}

			QString colName = part.mid(1, part.indexOf('`', 1) - 1);
			QString colType = part.mid(part.indexOf(' ') + 1, part.indexOf('(') - part.indexOf(' ') - 1).toLower();

			result.append(colName + " (" + colType + ")");
		}
	}
	else if (driver == "QPSQL")
	{
		QString tableSchema = table.split('.')[0];
		QString tableName = table.split('.')[1];

		query.prepare("SELECT \"column_name\",\"data_type\" FROM \"information_schema\".\"columns\" WHERE \"table_catalog\"='" + database + "' AND \"table_schema\"='" + tableSchema + "' AND \"table_name\"='" + tableName + "';");
		query.exec();

		while (query.next())
		{
			result.append(query.value(0).toString() + " (" + query.value(1).toString() + ")");
		}

		query.prepare("SELECT \"ordinal_position\" FROM \"information_schema\".\"key_column_usage\" WHERE \"table_catalog\"='" + database + "' AND \"table_schema\"='" + tableSchema + "' AND \"table_name\"='" + tableName + "' AND \"constraint_name\" LIKE '%_pkey';");
		query.exec();

		while (query.next())
		{
			QString primary = result.at(query.value(0).toInt());

			primary = primary.remove(primary.length() - 1, 1);
			primary += " P)";

			result.replace(query.value(0).toInt(), primary);
		}
	}

	return result;
}

QStringList Query::getEnumValues(QSqlDatabase* db, QString driver, QString database, QString table, QString column)
{
	QStringList result;
	QSqlQuery query(*db);

	if (driver == "QMYSQL")
	{
		query.prepare("SELECT `COLUMN_TYPE` FROM `information_schema`.`COLUMNS` WHERE `COLUMN_NAME`='" + column + "' AND `TABLE_NAME`='" + table + "' AND `TABLE_SCHEMA`='" + database + "';");
		query.exec();
		query.next();

		QString val = query.value(0).toString();
		val = val.remove(val.length() - 1, 1).remove(0, val.indexOf('(') + 1).replace("\'", "");

		result = val.split(',');
	}
	//Enums don't exist in SQLite
	//And I don't understand the PostgreSQL enums...

	return result;
}

QUERYRESULT Query::selectAll(QSqlDatabase* db, QString driver, QString database, QString table)
{
	QUERYRESULT result;
	QSqlQuery query(*db);

	if (driver == "QMYSQL")
	{
		query.prepare("SELECT * FROM `" + database + "`.`" + table + "`;");
	}
	else if (driver == "QSQLITE")
	{
		query.prepare("SELECT * FROM `" + table + "`;");
	}
	else if (driver == "QPSQL")
	{
		QString tableSchema = table.split('.')[0];
		QString tableName = table.split('.')[1];

		query.prepare("SELECT * FROM \"" + tableSchema + "\".\"" + tableName + "\";");
	}

	query.exec();

	while (query.next())
	{
		QList<QVariant> row;

		for (int i = 0; i < query.record().count(); i++)
		{
			row.append(query.value(i));
		}

		result.append(row);
	}

	return result;
}

QVariant Query::getVariant(QSqlDatabase* db, QString driver, QString database, QString table, QString column)
{
	QSqlQuery query(*db);

	if (driver == "QMYSQL")
	{
		query.prepare("SELECT `" + column + "` FROM `" + database + "`.`" + table + "`;");
	}
	else if (driver == "QSQLITE")
	{
		query.prepare("SELECT `" + column + "` FROM `" + table + "`;");
	}
	else if (driver == "QPSQL")
	{
		QString tableSchema = table.split('.')[0];
		QString tableName = table.split('.')[1];

		query.prepare("SELECT \"" + column + "\" FROM \"" + tableSchema + "\".\"" + tableName + "\";");
	}

	query.exec();
	query.next();

	return query.value(0);
}
#include <QDebug>
bool Query::updateTable(QSqlDatabase* db, QString driver, QString database, QString table, QMap<QString, QVariant>* update, QMap<QString, QVariant>* conditions)
{
	QSqlQuery query(*db);
	QString sql;

	if (driver == "QMYSQL")
	{
		sql += "UPDATE `" + database + "`.`" + table + "` SET ";

		for (int i = 0; i < update->size(); i++)
		{
			sql += "`" + update->keys()[i] + "`=?, ";
		}

		sql = sql.remove(sql.length() - 2, 2);
		sql += " WHERE ";

		for (int i = 0; i < conditions->size(); i++)
		{
			sql += "`" + conditions->keys()[i] + "`=? AND ";
		}

		sql = sql.remove(sql.length() - 5, 5);
		sql += ";";
	}
	else if (driver == "QSQLITE")
	{
		sql += "UPDATE `" + table + "` SET ";

		for (int i = 0; i < update->size(); i++)
		{
			sql += "`" + update->keys()[i] + "`=?, ";
		}

		sql = sql.remove(sql.length() - 2, 2);
		sql += " WHERE ";

		for (int i = 0; i < conditions->size(); i++)
		{
			sql += "`" + conditions->keys()[i] + "`=? AND ";
		}

		sql = sql.remove(sql.length() - 5, 5);
		sql += ";";
	}
	else if (driver == "QPSQL")
	{
		QString tableSchema = table.split('.')[0];
		QString tableName = table.split('.')[1];

		sql += "UPDATE \"" + tableSchema + "\".\"" + tableName + "\" SET ";

		for (int i = 0; i < update->size(); i++)
		{
			sql += "\"" + update->keys()[i] + "\"=?, ";
		}

		sql = sql.remove(sql.length() - 2, 2);
		sql += " WHERE ";

		for (int i = 0; i < conditions->size(); i++)
		{
			sql += "\"" + conditions->keys()[i] + "\"=? AND ";
		}

		sql = sql.remove(sql.length() - 5, 5);
		sql += ";";
	}

	query.prepare(sql);

	for (int i = 0; i < update->size(); i++)
	{
		query.addBindValue(update->values()[i]);
	}

	for (int i = 0; i < conditions->size(); i++)
	{
		query.addBindValue(conditions->values()[i]);
	}

	return query.exec();
}
