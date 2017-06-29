#ifndef FLATCOMBOBOX_HPP
#define FLATCOMBOBOX_HPP

#include <QComboBox>

class FlatComboBox : public QComboBox
{
	Q_OBJECT

public:
	explicit FlatComboBox(QWidget* parent = 0);

	~FlatComboBox();

protected:
	virtual void paintEvent(QPaintEvent*);
};

#endif // FLATCOMBOBOX_HPP
