#ifndef NEWCONNECTION_HPP
#define NEWCONNECTION_HPP

#include <QDialog>
#include <QMap>
#include <QString>

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

	void setValues(QMap<QString, QString> data);

	~NewConnection();

private:
	Ui::NewConnection* ui;
	QString oldName;
	void tryHost();

public slots:
	//TODO: Give better names
	void resetHost(QString);
	void tryDatabase();
	void tryName();
	void resetName(QString);
};

#endif // NEWCONNECTION_HPP
