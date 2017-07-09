#ifndef QUERY_HPP
#define QUERY_HPP

#include <QSqlDatabase>
#include <QStringList>

class Query
{
public:
	static QStringList listDatabases(QSqlDatabase* db, QString driver);
	static QStringList listTables(QSqlDatabase* db, QString driver, QString database);
};

#endif // QUERY_HPP
