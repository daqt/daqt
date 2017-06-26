#include "src/widgets/LongSpinBox.hpp"

#include <QLineEdit>

LongSpinBox::LongSpinBox(QWidget* parent) :
	QAbstractSpinBox(parent)
{
	connect(lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(setValue(QString)));
	connect(lineEdit(), SIGNAL(editingFinished()), this, SLOT(finished()));
}

void LongSpinBox::setMaximum(long long maximum)
{
	max = maximum;
}

void LongSpinBox::setMinimum(long long minimum)
{
	min = minimum;
}

void LongSpinBox::setValue(long long value)
{
	if (value > max)
	{
		val = max;
	}
	else if (value < min)
	{
		val = min;
	}
	else
	{
		val = value;
	}

	lineEdit()->setText(QString::number(val));
}

void LongSpinBox::stepBy(int steps)
{
	setValue(val + steps);
}

QValidator::State LongSpinBox::validate(QString &input, int &pos) const
{
	bool ok;
	long long value = input.toLongLong(&ok);

	if (!ok || value < min || value > max)
	{
		return QValidator::Invalid;
	}

	return QValidator::Acceptable;
}

QAbstractSpinBox::StepEnabled LongSpinBox::stepEnabled() const
{
	return StepUpEnabled | StepDownEnabled;
}

void LongSpinBox::stepUp()
{
	if (val + 1 < max)
	{
		val += 1;
	}

	lineEdit()->setText(QString::number(val));
}

void LongSpinBox::stepDown()
{
	if (val - 1 > min)
	{
		val -= 1;
	}

	lineEdit()->setText(QString::number(val));
}

void LongSpinBox::setValue(QString value)
{
	setValue(value.toLongLong());
}

void LongSpinBox::finished()
{
	emit editingFinished();
}

LongSpinBox::~LongSpinBox()
{

}
