#ifndef CONNECTIONTAB_HPP
#define CONNECTIONTAB_HPP

#include <QMap>
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

	void setConnectionData(QMap<QString, QString> data) { connectionData = data; }
	void loadDatabases();

	~ConnectionTab();

private:
	Ui::ConnectionTab* ui;

	QMap<QString, QString> connectionData;
};

#endif // DATABASETAB_HPP
