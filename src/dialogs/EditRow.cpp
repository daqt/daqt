#include "src/dialogs/EditRow.hpp"
#include "ui_EditRow.h"

#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QSqlDatabase>

#include "src/widgets/LongSpinBox.hpp"

EditRow::EditRow(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::EditRow)
{
	ui->setupUi(this);

	connect(ui->buttonCancel, SIGNAL(pressed()), this, SLOT(close()));
	connect(ui->buttonSave, SIGNAL(pressed()), this, SLOT(send()));
}

void EditRow::setItems(QList<QPair<QString, QPair<QVariant, bool>>>* items, bool newRow)
{
	QGridLayout* layout = new QGridLayout();

	for (int i = 0; i < items->length(); i++)
	{
		QLabel* label = new QLabel();
		label->setText(items->value(i).first);

		QCheckBox* nullBox = new QCheckBox();
		nullBox->setText("NULL");

		if (!items->value(i).second.second)
		{
			nullBox->setEnabled(false);
		}

		QFont font = nullBox->font();
		font.setItalic(true);
		nullBox->setFont(font);

		QVariant type = items->value(i).second.first;

		switch (type.type())
		{
		case type.DateTime:
		{
			QDateTimeEdit* edit = new QDateTimeEdit();
			QCalendarWidget* calendar = new QCalendarWidget(edit);

			calendar->setGridVisible(true);

			edit->setCalendarPopup(true);
			edit->setCalendarWidget(calendar);

			if (!type.isNull())
			{
				edit->setDateTime(type.toDateTime());
			}
			else
			{
				if (items->value(i).second.second)
				{
					nullBox->setChecked(true);
				}
				else
				{
					edit->setDateTime(QDateTime::currentDateTime());
				}
			}

			layout->addWidget(label, i, 0, 1, 1);
			layout->addWidget(edit, i, 1, 1, 1);
			layout->addWidget(nullBox, i, 2, 1, 1);
			break;
		}
		case type.Double:
		{
			QDoubleSpinBox* edit = new QDoubleSpinBox();

			edit->setValue(0.0);

			if (!type.isNull())
			{
				edit->setValue(type.toDouble());
			}
			else
			{
				if (items->value(i).second.second)
				{
					nullBox->setChecked(true);
				}
			}

			layout->addWidget(label, i, 0, 1, 1);
			layout->addWidget(edit, i, 1, 1, 1);
			layout->addWidget(nullBox, i, 2, 1, 1);
			break;
		}
		case type.UInt:
		case type.Int:
		case type.LongLong:
		case type.ULongLong:
		{
			LongSpinBox* edit = new LongSpinBox();

			if (type.type() == type.Int)
			{
				edit->setMaximum(INT_MAX);
				edit->setMinimum(INT_MIN);

				if (!type.isNull())
				{
					edit->setValue(type.toInt());
				}
			}
			else if (type.type() == type.UInt)
			{
				edit->setMaximum(UINT_MAX);
				edit->setMinimum(0);

				if (!type.isNull())
				{
					edit->setValue(type.toUInt());
				}
			}
			else if (type.type() == type.LongLong)
			{
				edit->setMaximum(LONG_LONG_MAX);
				edit->setMinimum(LONG_LONG_MIN);

				if (!type.isNull())
				{
					edit->setValue(type.toLongLong());
				}
			}
			else if (type.type() == type.ULongLong) //Not supported yet
			{
				edit->setMaximum(LONG_LONG_MAX);
				edit->setMinimum(0);

				if (!type.isNull())
				{
					edit->setValue(type.toULongLong());
				}
			}

			edit->setValue(0);

			if (type.isNull())
			{
				if (items->value(i).second.second)
				{
					nullBox->setChecked(true);
				}
			}

			edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred); //Required to use full width

			layout->addWidget(label, i, 0, 1, 1);
			layout->addWidget(edit, i, 1, 1, 1);
			layout->addWidget(nullBox, i, 2, 1, 1);
			break;
		}
		case type.StringList:
		{
			QComboBox* edit = new QComboBox();

			QStringList values = type.toStringList();

			QString defaultValue = values[0];
			int defaultIndex = QString(defaultValue.split(".").last()).toInt();

			values[0] = values[defaultIndex];
			values[defaultIndex] = defaultValue.mid(0, defaultValue.lastIndexOf("."));

			edit->addItems(values);

			edit->setCurrentIndex(defaultIndex);

			if (type.isNull())
			{
				if (items->value(i).second.second)
				{
					nullBox->setChecked(true);
				}
			}

			layout->addWidget(label, i, 0, 1, 1);
			layout->addWidget(edit, i, 1, 1, 1);
			layout->addWidget(nullBox, i, 2, 1, 1);
			break;
		}
		default:
			QLineEdit* edit = new QLineEdit();

			if (!type.isNull())
			{
				edit->setText(type.toString());
			}
			else
			{
				if (items->value(i).second.second)
				{
					nullBox->setChecked(true);
				}
			}

			layout->addWidget(label, i, 0, 1, 1);
			layout->addWidget(edit, i, 1, 1, 1);
			layout->addWidget(nullBox, i, 2, 1, 1);
			break;
		}
	}

	ui->areaWidgets->setLayout(layout);

	if (newRow)
	{
		this->setWindowTitle("Insert Row");
		ui->buttonSave->setText("Insert");
	}
}

void EditRow::send()
{
	QList<QPair<QString, QVariant>>* data = new QList<QPair<QString, QVariant>>();

	for (int i = 0; i < ui->areaWidgets->layout()->count(); i += 3) //Go to next row
	{
		QString name = qobject_cast<QLabel*>(ui->areaWidgets->layout()->itemAt(i)->widget())->text();
		QVariant value;

		if (qobject_cast<QCheckBox*>(ui->areaWidgets->layout()->itemAt(i + 2)->widget())->isChecked())
		{
			value = QVariant(QString()); //NULL
		}
		else
		{
			QWidget* widget = ui->areaWidgets->layout()->itemAt(i + 1)->widget();

			if (qobject_cast<LongSpinBox*>(widget))
			{
				value = QVariant(qobject_cast<LongSpinBox*>(widget)->value());
			}
			else if (qobject_cast<QDoubleSpinBox*>(widget))
			{
				value = QVariant(qobject_cast<QDoubleSpinBox*>(widget)->value());
			}
			else if (qobject_cast<QDateTimeEdit*>(widget))
			{
				value = QVariant(qobject_cast<QDateTimeEdit*>(widget)->dateTime());
			}
			else if (qobject_cast<QComboBox*>(widget))
			{
				value = QVariant(qobject_cast<QComboBox*>(widget)->currentText());
			}
			else
			{
				value = QVariant(qobject_cast<QLineEdit*>(widget)->text());
			}
		}

		data->append(QPair<QString, QVariant>(name, value));
	}

	emit save(data);

	this->accept();
}

EditRow::~EditRow()
{
	delete ui;
}
