#ifndef PASSWORD_HPP
#define PASSWORD_HPP

#include <QDialog>

namespace Ui {
	class Password;
}

class Password : public QDialog
{
	Q_OBJECT

public:
	explicit Password(QWidget* parent = 0);

	~Password();

private:
	Ui::Password* ui;

signals:
	void sendPassword(QString password, bool save);

public slots:
	void send();
};

#endif // PASSWORD_HPP
