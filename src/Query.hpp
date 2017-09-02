#ifndef QUERY_HPP
#define QUERY_HPP

#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QStringList>
#include <QVariant>

#define QUERYRESULT QList<QList<QVariant>>

class Query
{
public:
	static QStringList listDatabases(QSqlDatabase* db, QString driver);
	static QStringList listTables(QSqlDatabase* db, QString driver, QString database);
	static QStringList getHeader(QSqlDatabase* db, QString driver, QString database, QString table);
	static QStringList getEnumValues(QSqlDatabase* db, QString driver, QString database, QString table, QString column);

	static QUERYRESULT selectAll(QSqlDatabase* db, QString driver, QString database, QString table, int page);
	static QVariant getVariant(QSqlDatabase* db, QString driver, QString database, QString table, QString column, int row = -1);
	static bool updateRow(QSqlDatabase* db, QString driver, QString database, QString table, QMap<QString, QVariant>* update, QMap<QString, QVariant>* conditions);
	static int getRows(QSqlDatabase* db, QString driver, QString database, QString table);
	static QString getDefault(QSqlDatabase* db, QString driver, QString database, QString table, QString column);
	static bool insertRow(QSqlDatabase* db, QString driver, QString database, QString table, QMap<QString, QVariant>* values);
	static bool getNullable(QSqlDatabase* db, QString driver, QString database, QString table, QString column);
};

#endif // QUERY_HPP
