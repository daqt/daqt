#include "src/widgets/ConnectionTab.hpp"
#include "ui_ConnectionTab.h"

#include <QCalendarWidget>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDir>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QtMath>
#include <QScrollBar>
#include <QSignalMapper>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTableWidgetItem>
#include <QThread>

#include "src/dialogs/Password.hpp"
#include "src/Query.hpp"
#include "src/widgets/FlatComboBox.hpp"
#include "src/widgets/LongSpinBox.hpp"

ConnectionTab::ConnectionTab(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::ConnectionTab)
{
	ui->setupUi(this);

	connect(ui->listDatabases, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(loadTables(QModelIndex)));
	connect(ui->listTables, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openTable(QModelIndex)));
	connect(ui->tableValues, SIGNAL(cellChanged(int, int)), this, SLOT(changeValue(int, int)));
	connect(ui->tableValues, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(handleType(int, int)));

	connect(ui->buttonNext, SIGNAL(pressed()), ui->spinPage, SLOT(stepUp()));
	connect(ui->buttonPrevious, SIGNAL(pressed()), ui->spinPage, SLOT(stepDown()));
	connect(ui->spinPage, SIGNAL(valueChanged(int)), this, SLOT(goPage(int)));

	goPage(0);
}

void ConnectionTab::setConnectionData(Connection* connection)
{
	connectionData = connection;

	if (connectionData == NULL || !connectionData->isValid())
	{
		return;
	}

	db = QSqlDatabase::addDatabase(connectionData->getDriver(), connectionData->getName());

	driver = connectionData->getDriver();

	if (connectionData->getDriver() != "QSQLITE")
	{
		db.setHostName(connectionData->getHost().host());
		db.setPort(connectionData->getHost().port());
		db.setUserName(connectionData->getUsername());

		if (!connectionData->getPassword().isNull())
		{
			db.setPassword(connectionData->getPassword());
		}
		else
		{
			requestPassword();
		}
	}
	else
	{
		db.setDatabaseName(connection->getPath());
	}
}

void ConnectionTab::loadDatabases()
{
	ui->listDatabases->clear();

	if (db.open())
	{
		QStringList databases = Query::listDatabases(&db, driver);

		for (int i = 0; i < databases.length(); i++)
		{
			ui->listDatabases->addItem(databases[i]);
		}

		db.close();
	}
}

bool ConnectionTab::canConnect()
{
	if (db.open())
	{
		db.close();

		return true;
	}

	return false;
}

QString ConnectionTab::getError()
{
	return db.lastError().databaseText();
}

void ConnectionTab::finish()
{
	emit finished();
	this->close();
}

void ConnectionTab::loadTables(QModelIndex index)
{
	if (db.open())
	{
		goPage(0);

		ui->listTables->clear();
		ui->listTables->verticalScrollBar()->setValue(0);

		ui->tableValues->setRowCount(0);
		ui->tableValues->setColumnCount(0);
		ui->tableValues->clearContents();

		databaseName = index.data().toString();

		if (driver != "QSQLITE")
		{
			if (driver == "QPSQL" && db.databaseName() != databaseName) //PostgreSQL needs you to doubleclick the database twice, this fixes it
			{
				db.setDatabaseName(databaseName);
				loadTables(index);
				ui->listTables->clear();
				db.open();
			}

			db.setDatabaseName(databaseName);
		}

		QStringList tables = Query::listTables(&db, driver, databaseName);

		for (int i = 0; i < tables.length(); i++)
		{
			ui->listTables->addItem(tables[i]);
		}

		db.close();
	}
}

void ConnectionTab::openTable(QModelIndex index)
{
	ui->tableValues->blockSignals(true);

	ui->tableValues->setRowCount(0);
	ui->tableValues->setColumnCount(0);
	ui->tableValues->clearContents();
	ui->tableValues->horizontalScrollBar()->setValue(0);
	ui->tableValues->verticalScrollBar()->setValue(0);

	if (db.open())
	{
		tableName = index.data().toString();

		QStringList header = Query::getHeader(&db, driver, databaseName, tableName);

		ui->tableValues->setColumnCount(header.length());
		ui->tableValues->setHorizontalHeaderLabels(header);

		for (int i = 0; i < ui->tableValues->horizontalHeader()->count(); i++)
		{
			if (ui->tableValues->horizontalHeaderItem(i)->text().endsWith(" P)"))
			{
				QTableWidgetItem* primary = ui->tableValues->horizontalHeaderItem(i);
				primary->setTextColor(QColor().fromRgb(0xDD, 0xDD, 0x00));
				ui->tableValues->setHorizontalHeaderItem(i,  primary);
			}
			else //Required since colors are not reset
			{
				QTableWidgetItem* notPrimary = ui->tableValues->horizontalHeaderItem(i);
				notPrimary->setTextColor(QPalette().foreground().color());
				ui->tableValues->setHorizontalHeaderItem(i, notPrimary);
			}
		}

		QSignalMapper* mapper = new QSignalMapper();
		QUERYRESULT all = Query::selectAll(&db, driver, databaseName, tableName, 1);

		for (int y = 0; y < all.length(); y++)
		{
			ui->tableValues->insertRow(ui->tableValues->rowCount());

			for (int x = 0; x < all[y].length(); x++)
			{
				ui->tableValues->setItem(ui->tableValues->rowCount() - 1, x, new QTableWidgetItem(all[y][x].toString()));

				if (all[y][x].toString().isEmpty())
				{
					ui->tableValues->item(ui->tableValues->rowCount() - 1, x)->setText("NULL");

					QFont font = QFont(ui->tableValues->item(ui->tableValues->rowCount() - 1, x)->font());
					font.setItalic(true);
					ui->tableValues->item(ui->tableValues->rowCount() - 1, x)->setFont(font);

					continue;
				}

				handleType(ui->tableValues->rowCount() - 1, x, all[y][x]);
			}
		}

		connect(mapper, SIGNAL(mapped(QString)), this, SLOT(editFinished(QString)));

		int max = ceil(Query::getRows(&db, driver, databaseName, tableName) / 50);

		if (max == 0)
		{
			max = 1;
		}

		ui->spinPage->setMaximum(max);
		ui->spinPage->setSuffix("/" + QString::number(max));

		goPage(1);

		db.close();
	}

	ui->tableValues->resizeColumnsToContents();

	ui->tableValues->blockSignals(false);
}

void ConnectionTab::changeValue(int row, int column)
{
	if (qobject_cast<QLineEdit*>(ui->tableValues->cellWidget(row, column)) != NULL)
	{
		QString str;
		str += QString::number(row);
		str += ",";
		str += QString::number(column);

		editFinished(str);
	}
}

void ConnectionTab::setPassword(QString password, bool save)
{
	db.setPassword(password);

	if (save)
	{
		connectionData->setPassword(password);
		connectionData->save();
	}
}

void ConnectionTab::requestPassword()
{
	Password* password = new Password();

	connect(password, SIGNAL(sendPassword(QString, bool)), this, SLOT(setPassword(QString, bool)));
	connect(password, SIGNAL(finished(int)), this, SLOT(open(int)));

	password->setModal(true);
	password->exec();
}

void ConnectionTab::handleError()
{
	setStatusTip(getError());

	if (driver == "QMYSQL")
	{
		switch (db.lastError().nativeErrorCode().toInt())
		{
		case 1045:
			requestPassword();
			break;
		}
	}
}

void ConnectionTab::open(int code)
{
	if (canConnect())
	{
		loadDatabases();
	}
	else
	{
		if (code > 0)
		{
			handleError();
		}
		else
		{
			finish();
		}
	}
}

void ConnectionTab::handleType(int row, int column)
{
	QVariant variant;

	if (db.open())
	{
		QString columnName = ui->tableValues->horizontalHeaderItem(column)->text().split(" ")[0];

		variant = Query::getVariant(&db, driver, databaseName, tableName, columnName);

		db.close();
	}

	handleType(row, column, variant);
}

void ConnectionTab::handleType(int row, int column, QVariant type)
{
	QSignalMapper* mapper = new QSignalMapper();

	QString columnName = ui->tableValues->horizontalHeaderItem(column)->text().split("(")[0];
	QString columnType = QString(ui->tableValues->horizontalHeaderItem(column)->text().split("(")[1]);
	columnType = columnType.remove(columnType.length() - 1, 1);
	columnType = columnType.split(" ")[0];

	if (columnType == "enum")
	{
		FlatComboBox* edit = new FlatComboBox(ui->tableValues);

		if (row % 2 == 1)
		{
			QColor background = QPalette().alternateBase().color();
			edit->setStyleSheet("background-color: rgb(" + QString::number(background.red()) + "," + QString::number(background.green()) + "," + QString::number(background.blue()) + ")");
		}

		if (db.open())
		{
			edit->addItems(Query::getEnumValues(&db, driver, databaseName, tableName, columnName));

			db.close();
		}

		QString str;
		str += QString::number(row);
		str += ",";
		str += QString::number(column);

		connect(edit, SIGNAL(currentIndexChanged(int)), mapper, SLOT(map()));
		mapper->setMapping(edit, str);

		ui->tableValues->setCellWidget(row, column, edit);
	}

	switch (type.type())
	{
	case type.DateTime:
	{
		QDateTimeEdit* edit = new QDateTimeEdit(ui->tableValues);
		QCalendarWidget* calendar = new QCalendarWidget(edit);

		calendar->setGridVisible(true);

		if (row % 2 == 1)
		{
			QColor background = QPalette().alternateBase().color();
			edit->setStyleSheet("background-color: rgb(" + QString::number(background.red()) + "," + QString::number(background.green()) + "," + QString::number(background.blue()) + ")");
		}

		edit->setCalendarPopup(true);
		edit->setCalendarWidget(calendar);

		if (type.toString().isEmpty())
		{
			edit->setDateTime(QDateTime().currentDateTime());
		}
		else
		{
			edit->setDateTime(type.toDateTime());
		}

		QString str;
		str += QString::number(row);
		str += ",";
		str += QString::number(column);

		connect(edit, SIGNAL(editingFinished()), mapper, SLOT(map()));
		mapper->setMapping(edit, str);

		ui->tableValues->setCellWidget(row, column, edit);
		break;
	}
	case type.Double:
	{
		QDoubleSpinBox* edit = new QDoubleSpinBox(ui->tableValues);

		if (row % 2 == 1)
		{
			QColor background = QPalette().alternateBase().color();
			edit->setStyleSheet("background-color: rgb(" + QString::number(background.red()) + "," + QString::number(background.green()) + "," + QString::number(background.blue()) + ")");
		}

		edit->setValue(type.toDouble());

		QString str;
		str += QString::number(row);
		str += ",";
		str += QString::number(column);

		connect(edit, SIGNAL(editingFinished()), mapper, SLOT(map()));
		mapper->setMapping(edit, str);

		ui->tableValues->setCellWidget(row, column, edit);
		break;
	}
	case type.UInt:
	case type.Int:
	case type.LongLong:
	case type.ULongLong:
	{
		LongSpinBox* edit = new LongSpinBox(ui->tableValues);

		if (row % 2 == 1)
		{
			QColor background = QPalette().alternateBase().color();
			edit->setStyleSheet("background-color: rgb(" + QString::number(background.red()) + "," + QString::number(background.green()) + "," + QString::number(background.blue()) + ")");
		}

		if (type.type() == type.Int)
		{
			edit->setMaximum(INT_MAX);
			edit->setMinimum(INT_MIN);
			edit->setValue(type.toInt());
		}
		else if (type.type() == type.UInt)
		{
			edit->setMaximum(UINT_MAX);
			edit->setMinimum(0);
			edit->setValue(type.toUInt());
		}
		else if (type.type() == type.LongLong)
		{
			edit->setMaximum(LONG_LONG_MAX);
			edit->setMinimum(LONG_LONG_MIN);
			edit->setValue(type.toLongLong());
		}
		else if (type.type() == type.ULongLong) //Not supported yet
		{
			edit->setMaximum(LONG_LONG_MAX);
			edit->setMinimum(0);
			edit->setValue(type.toULongLong());
		}

		QString str;
		str += QString::number(row);
		str += ",";
		str += QString::number(column);

		connect(edit, SIGNAL(editingFinished()), mapper, SLOT(map()));
		mapper->setMapping(edit, str);

		ui->tableValues->setCellWidget(row, column, edit);
		break;
	}
	default:
		break;
	}

	connect(mapper, SIGNAL(mapped(QString)), this, SLOT(editFinished(QString)));
}

void ConnectionTab::editFinished(QString data)
{
	if (db.open())
	{
		int row = QString(data.split(",")[0]).toInt();
		int column = QString(data.split(",")[1]).toInt();

		QString columnName = ui->tableValues->horizontalHeaderItem(column)->text().split(' ')[0];
		QString columnType = QString(ui->tableValues->horizontalHeaderItem(column)->text().split(' ')[1]).replace("(", "").replace(")", "");
		QVariant type = Query::getVariant(&db, driver, databaseName, tableName, columnName);

		QMap<QString, QVariant>* conditions = new QMap<QString, QVariant>();

		for (int i = 0; i < ui->tableValues->horizontalHeader()->count(); i++)
		{
			if (i != column)
			{
				conditions->insert(ui->tableValues->horizontalHeaderItem(i)->text().split(' ')[0], Query::getVariant(&db, driver, databaseName, tableName, ui->tableValues->horizontalHeaderItem(i)->text().split(' ')[0]));
			}
		}

		QMap<QString, QVariant>* values = new QMap<QString, QVariant>();

		if (columnType == "enum")
		{
			FlatComboBox* edit = qobject_cast<FlatComboBox*>(ui->tableValues->cellWidget(row, column));

			ui->tableValues->item(row, column)->setText(edit->currentText());

			values->insert(columnName, QVariant(edit->currentText()));
		}

		switch (type.type())
		{
		case type.DateTime:
		{
			QDateTimeEdit* edit = qobject_cast<QDateTimeEdit*>(ui->tableValues->cellWidget(row, column));

			values->insert(columnName, QVariant(edit->dateTime().toString(Qt::ISODate)));
			break;
		}
		case type.Double:
		{
			QDoubleSpinBox* edit = qobject_cast<QDoubleSpinBox*>(ui->tableValues->cellWidget(row, column));

			values->insert(columnName, QVariant(edit->text()));
			break;
		}
		case type.Int:
		case type.UInt:
		case type.LongLong:
		case type.ULongLong:
		{
			LongSpinBox* edit = qobject_cast<LongSpinBox*>(ui->tableValues->cellWidget(row, column));

			values->insert(columnName, QVariant(edit->text()));
			break;
		}
		default:
			break;
		}

		if (values->size() == 0)
		{
			values->insert(columnName, QVariant(ui->tableValues->item(row, column)->text()));
		}

		Query::updateTable(&db, driver, databaseName, tableName, values, conditions);

		db.close();
	}
}

void ConnectionTab::goPage(int page)
{
	if (page > 0)
	{
		ui->spinPage->setValue(page);
		ui->spinPage->setMinimum(1);

		ui->tableValues->setRowCount(0);
		ui->tableValues->clearContents();
		ui->tableValues->verticalScrollBar()->setValue(0);

		if (db.open())
		{
			QSignalMapper* mapper = new QSignalMapper();
			QUERYRESULT all = Query::selectAll(&db, driver, databaseName, tableName, page);

			for (int y = 0; y < all.length(); y++)
			{
				ui->tableValues->insertRow(ui->tableValues->rowCount());

				for (int x = 0; x < all[y].length(); x++)
				{
					ui->tableValues->setItem(ui->tableValues->rowCount() - 1, x, new QTableWidgetItem(all[y][x].toString()));

					if (all[y][x].toString().isEmpty())
					{
						ui->tableValues->item(ui->tableValues->rowCount() - 1, x)->setText("NULL");

						QFont font = QFont(ui->tableValues->item(ui->tableValues->rowCount() - 1, x)->font());
						font.setItalic(true);
						ui->tableValues->item(ui->tableValues->rowCount() - 1, x)->setFont(font);

						continue;
					}

					handleType(ui->tableValues->rowCount() - 1, x, all[y][x]);
				}
			}

			connect(mapper, SIGNAL(mapped(QString)), this, SLOT(editFinished(QString)));

			db.close();
		}
	}
	else
	{
		ui->spinPage->blockSignals(true);

		ui->spinPage->setMinimum(0);
		ui->spinPage->setValue(0);
		ui->spinPage->setMaximum(0);
		ui->spinPage->setSuffix("/0");

		ui->spinPage->blockSignals(false);
	}

	if (page <= 1)
	{
		ui->buttonPrevious->setEnabled(false);
	}
	else
	{
		ui->buttonPrevious->setEnabled(true);
	}

	if (page >= ui->spinPage->maximum() || ui->spinPage->maximum() <= 1)
	{
		ui->buttonNext->setEnabled(false);
	}
	else
	{
		ui->buttonNext->setEnabled(true);
	}
}

ConnectionTab::~ConnectionTab()
{
	db = QSqlDatabase();
	QSqlDatabase::removeDatabase(connectionData->getName());

	delete ui;
}
