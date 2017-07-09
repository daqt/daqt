#ifndef NEWCONNECTION_HPP
#define NEWCONNECTION_HPP

#include <QDialog>
#include <QMap>
#include <QString>

#include "src/types/Connection.hpp"

#define DIALOG_NEWCONNECTION  0x0D01
#define DIALOG_EDITCONNECTION 0x0D02

namespace Ui {
	class NewConnection;
}

class NewConnection : public QDialog
{
	Q_OBJECT

public:
	explicit NewConnection(QWidget* parent = 0);

	void setValues(Connection* connection);

	~NewConnection();

private:
	Ui::NewConnection* ui;
	QString oldName;

public slots:
	void resetHostColor(QString);
	void resetNameColor(QString);
	void tryConnect();
	void checkName();
	void checkFile(QString);
	void changeDriver(QString driver);
	void browseFile(bool);
};

#endif // NEWCONNECTION_HPP
