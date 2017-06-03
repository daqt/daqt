#ifndef DATABASETAB_HPP
#define DATABASETAB_HPP

#include <QMap>
#include <QString>
#include <QWidget>

namespace Ui {
	class DatabaseTab;
}

class DatabaseTab : public QWidget
{
	Q_OBJECT

public:
	explicit DatabaseTab(QWidget* parent = 0);
	~DatabaseTab();

	void setDatabaseData(QMap<QString, QString> data) { databaseData = data; }

	void loadDatabases();

private:
	Ui::DatabaseTab* ui;

	QMap<QString, QString> databaseData;
};

#endif // DATABASETAB_HPP
