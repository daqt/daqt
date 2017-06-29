#include "FlatComboBox.hpp"

#include <QLineEdit>
#include <QStylePainter>

FlatComboBox::FlatComboBox(QWidget* parent) :
	QComboBox(parent)
{

}

void FlatComboBox::paintEvent(QPaintEvent*)
{
	QStylePainter painter(this);
	QStyleOptionComboBox option;

	initStyleOption(&option);

	QRect rect(option.rect);
	option.currentText = "";

	painter.setPen(palette().color(QPalette::Text));
	option.rect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignRight, QSize(16, rect.height()), rect);
	painter.drawPrimitive(QStyle::PE_IndicatorArrowDown, option);
	option.rect = rect;
	painter.drawControl(QStyle::CE_ComboBoxLabel, option);
}

FlatComboBox::~FlatComboBox()
{

}
