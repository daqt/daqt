#ifndef CONNECTIONTAB_HPP
#define CONNECTIONTAB_HPP

#include <QMap>
#include <QModelIndex>
#include <QSqlDatabase>
#include <QString>
#include <QWidget>

namespace Ui {
	class ConnectionTab;
}

class ConnectionTab : public QWidget
{
	Q_OBJECT

public:
	explicit ConnectionTab(QWidget* parent = 0);

	void setConnectionData(QMap<QString, QString> data);
	void loadDatabases();

	~ConnectionTab();

private:
	Ui::ConnectionTab* ui;
	QMap<QString, QString> connectionData;
	QSqlDatabase db;
	QString databaseName, tableName;

public slots:
	void loadTables(QModelIndex index);
	void openTable(QModelIndex index);
	void changeValue(int row, int column);
};

#endif // DATABASETAB_HPP
