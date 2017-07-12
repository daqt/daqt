#ifndef CONNECTIONTAB_HPP
#define CONNECTIONTAB_HPP

#include <QMap>
#include <QModelIndex>
#include <QSqlDatabase>
#include <QString>
#include <QWidget>

#include "src/types/Connection.hpp"

namespace Ui {
	class ConnectionTab;
}

class ConnectionTab : public QWidget
{
	Q_OBJECT
	
public:
	explicit ConnectionTab(QWidget* parent = 0);
	
	void setConnectionData(Connection* connection);
	void loadDatabases();
	
	bool canConnect();
	QString getError();
	
	void finish();
	
	~ConnectionTab();
	
private:
	Ui::ConnectionTab* ui;
	Connection* connectionData;
	QSqlDatabase db;
	QString databaseName, tableName, driver;
	
	void requestPassword();
	void handleError();
	
signals:
	void finished();
	
public slots:
	void loadTables(QModelIndex index);
	void openTable(QModelIndex index);
	void changeValue(int row, int column);
	void setPassword(QString password, bool save);
	void open(int code);
	void handleType(int row, int column, QVariant type);
	void editFinished(QString data);
};

#endif // DATABASETAB_HPP
