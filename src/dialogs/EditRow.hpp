#ifndef EDITROW_HPP
#define EDITROW_HPP

#include <QDialog>
#include <QList>
#include <QString>
#include <QVariant>

namespace Ui {
	class EditRow;
}

class EditRow : public QDialog
{
	Q_OBJECT

public:
	explicit EditRow(QWidget* parent = 0);

	void setItems(QList<QPair<QString, QPair<QVariant, bool>>>* items, bool newRow = false);

	~EditRow();

private:
	Ui::EditRow *ui;

signals:
	void save(QList<QPair<QString, QVariant>>* data);

public slots:
	void send();
};

#endif // EDITROW_HPP
