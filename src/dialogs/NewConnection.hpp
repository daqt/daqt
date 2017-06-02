#ifndef NEWCONNECTION_HPP
#define NEWCONNECTION_HPP

#include <QDialog>

#define DIALOG_NEWCONNECTION 0x0D01

namespace Ui {
	class NewConnection;
}

class NewConnection : public QDialog
{
	Q_OBJECT

public:
	explicit NewConnection(QWidget* parent = 0);
	~NewConnection();

private:
	Ui::NewConnection* ui;
	void tryHost();

public slots:
	//TODO: Give better names
	void resetHost(QString);
	void tryDatabase();
	void tryName();
	void resetName(QString);
};

#endif // NEWCONNECTION_HPP
