#ifndef LONGSPINBOX_H
#define LONGSPINBOX_H

#include <QAbstractSpinBox>
#include <QWidget>

class LongSpinBox : public QAbstractSpinBox
{
	Q_OBJECT

public:
	explicit LongSpinBox(QWidget* parent = 0);

	void setMaximum(long long maximum);
	void setMinimum(long long minimum);

	void setValue(long long value);

	void stepBy(int steps);

	~LongSpinBox();

protected:
	QValidator::State validate(QString &input, int &pos) const;
	QAbstractSpinBox::StepEnabled stepEnabled() const;

private:
	long long min;
	long long max;
	long long val;

signals:
	void editingFinished();

public slots:
	void stepUp();
	void stepDown();

	void setValue(QString value);

	void finished();
};

#endif // LONGSPINBOX_H
