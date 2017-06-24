#ifndef ABOUT_HPP
#define ABOUT_HPP

#include <QDialog>

#define DIALOG_ABOUT 0x0D03

namespace Ui {
	class About;
}

class About : public QDialog
{
	Q_OBJECT

public:
	explicit About(QWidget* parent = 0);
	~About();

private:
	Ui::About* ui;
};

#endif // ABOUT_HPP
