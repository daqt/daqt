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

public slots:
	void tryHost();
	void resetHost(QString); //TODO: Give a better name
	void tryDatabase();
};

#endif // NEWCONNECTION_HPP
