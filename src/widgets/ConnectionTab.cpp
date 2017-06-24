#include "src/widgets/ConnectionTab.hpp"
#include "ui_ConnectionTab.h"

#include <QCalendarWidget>
#include <QDateTimeEdit>
#include <QSignalMapper>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTableWidgetItem>
#include <QThread>

#include "src/dialogs/Password.hpp"

ConnectionTab::ConnectionTab(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::ConnectionTab)
{
	ui->setupUi(this);

	connect(ui->listDatabases, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(loadTables(QModelIndex)));
	connect(ui->listTables, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openTable(QModelIndex)));
	connect(ui->tableValues, SIGNAL(cellChanged(int, int)), this, SLOT(changeValue(int, int)));
	connect(ui->tableValues, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(handleType(int, int)));
}

void ConnectionTab::setConnectionData(Connection* connection)
{
	connectionData = connection;

	if (connectionData == NULL || !connectionData->isValid())
	{
		return;
	}

	db = QSqlDatabase::addDatabase(connectionData->getDriver(), connectionData->getName());
	db.setHostName(connectionData->getHost().host());
	db.setPort(connectionData->getHost().port());
	db.setUserName(connectionData->getUsername());

	driver = connectionData->getDriver();

	if (!connectionData->getPassword().isNull())
	{
		db.setPassword(connectionData->getPassword());
	}
	else
	{
		requestPassword();
	}
}

void ConnectionTab::loadDatabases()
{
	if (db.open())
	{
		if (driver == "QMYSQL")
		{
			QSqlQuery query(db);
			query.prepare("SELECT `SCHEMA_NAME` FROM `information_schema`.`SCHEMATA`");
			query.exec();

			while (query.next())
			{
				ui->listDatabases->addItem(query.value(0).toString());
			}
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
		ui->listTables->clear();

		databaseName = index.data().toString();

		if (driver == "QMYSQL")
		{
			db.setDatabaseName(databaseName);

			QSqlQuery query(db);
			query.prepare("SELECT `TABLE_NAME` FROM `information_schema`.`TABLES` WHERE `TABLE_SCHEMA`='" + databaseName + "';");
			query.exec();

			while (query.next())
			{
				ui->listTables->addItem(query.value(0).toString());
			}
		}

		db.close();
	}
}

void ConnectionTab::openTable(QModelIndex index)
{
	ui->tableValues->blockSignals(true);

	ui->tableValues->setRowCount(0);
	ui->tableValues->clearContents();

	if (db.open())
	{
		tableName = index.data().toString();

		if (driver == "QMYSQL")
		{
			QSqlQuery query(db);

			query.prepare("SELECT `COLUMN_NAME`,`DATA_TYPE` FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA`='" + db.databaseName() + "' AND `TABLE_NAME`='" + tableName + "';");
			query.exec();

			QStringList header;

			while (query.next())
			{
				header.append(query.value(0).toString() + " (" + query.value(1).toString() + ")");
			}

			ui->tableValues->setColumnCount(header.length());
			ui->tableValues->setHorizontalHeaderLabels(header);

			query.prepare("SELECT `ORDINAL_POSITION` FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA`='" + db.databaseName() + "' AND `TABLE_NAME`='" + tableName + "' AND `CONSTRAINT_NAME`='PRIMARY';");
			query.exec();

			while (query.next())
			{
				QTableWidgetItem* primary = ui->tableValues->horizontalHeaderItem(query.value(0).toInt() - 1);
				primary->setTextColor(QColor().fromRgb(0xDD, 0xDD, 0x00));
				ui->tableValues->setHorizontalHeaderItem(query.value(0).toInt() - 1, primary);
			}

			query.prepare("SELECT * FROM `" + tableName + "`");
			query.exec();

			QSignalMapper* mapper = new QSignalMapper();

			while (query.next())
			{
				ui->tableValues->insertRow(ui->tableValues->rowCount());

				for (int i = 0; i < query.record().count(); i++)
				{
					ui->tableValues->setItem(ui->tableValues->rowCount() - 1, i, new QTableWidgetItem(query.value(i).toString()));

					if (query.value(i).toString().isEmpty())
					{
						ui->tableValues->item(ui->tableValues->rowCount() - 1, i)->setText("NULL");

						QFont font = QFont(ui->tableValues->item(ui->tableValues->rowCount() - 1, i)->font());
						font.setItalic(true);
						ui->tableValues->item(ui->tableValues->rowCount() - 1, i)->setFont(font);

						continue;
					}

					QVariant type = query.record().field(i).type();

					if (type.type() == type.DateTime)
					{
						QDateTimeEdit* edit = new QDateTimeEdit(ui->tableValues);
						QCalendarWidget* calendar = new QCalendarWidget(edit);

						calendar->setGridVisible(true);

						if ((ui->tableValues->rowCount() - 1) % 2 == 1)
						{
							QColor background = QPalette().alternateBase().color();
							edit->setStyleSheet("background-color: rgb(" + QString::number(background.red()) + "," + QString::number(background.green()) + "," + QString::number(background.blue()) + ")");
						}

						edit->setCalendarPopup(true);
						edit->setCalendarWidget(calendar);
						edit->setDateTime(query.value(i).toDateTime());

						QString str;
						str += QString::number(ui->tableValues->rowCount() - 1);
						str += ",";
						str += QString::number(i);

						connect(edit, SIGNAL(editingFinished()), mapper, SLOT(map()));
						mapper->setMapping(edit, str);

						ui->tableValues->setCellWidget(ui->tableValues->rowCount() - 1, i, edit);
					}
				}
			}

			connect(mapper, SIGNAL(mapped(QString)), this, SLOT(editFinished(QString)));

			ui->tableValues->resizeColumnsToContents();
		}

		db.close();
	}

	ui->tableValues->blockSignals(false);
}

void ConnectionTab::changeValue(int row, int column)
{
	if (db.open())
	{
		QString primary = "id";
		int primaryIndex = 0;

		if (driver == "QMYSQL")
		{
			QSqlQuery query(db);

			query.prepare("SELECT `COLUMN_NAME`,`ORDINAL_POSITION` FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA`='" + db.databaseName() + "' AND `TABLE_NAME`='" + tableName + "' AND `CONSTRAINT_NAME`='PRIMARY';");
			query.exec();
			query.next();

			primary = query.value(0).toString();
			primaryIndex = query.value(1).toInt() - 1;

			QString columnName = ui->tableValues->horizontalHeaderItem(column)->text().split(' ')[0];
			QString primaryVal = ui->tableValues->item(row, primaryIndex)->text();

			query.prepare("UPDATE `" + tableName + "` SET `" + columnName + "`=? WHERE `" + primary + "`='" + primaryVal + "'");
			query.addBindValue(ui->tableValues->item(row, column)->text());
			query.exec();
		}

		db.close();
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
	if (db.open())
	{
		QString columnName = ui->tableValues->horizontalHeaderItem(column)->text().split(' ')[0];

		QSignalMapper* mapper = new QSignalMapper();

		if (driver == "QMYSQL")
		{
			QSqlQuery query(db);

			query.prepare("SELECT `" + columnName + "` FROM `" + tableName + "` LIMIT 1");
			query.exec();
			query.next();

			QVariant type = query.record().field(columnName).type();

			if (type.type() == type.DateTime)
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
				edit->setDateTime(QDateTime().currentDateTime());

				QString str;
				str += QString::number(row);
				str += ",";
				str += QString::number(column);

				connect(edit, SIGNAL(editingFinished()), mapper, SLOT(map()));
				mapper->setMapping(edit, str);

				ui->tableValues->setCellWidget(row, column, edit);
			}

			connect(mapper, SIGNAL(mapped(QString)), this, SLOT(editFinished(QString)));
		}

		db.close();
	}
}

void ConnectionTab::editFinished(QString data)
{
	int row = QString(data.split(",")[0]).toInt();
	int column = QString(data.split(",")[1]).toInt();

	if (db.open())
	{
		QString columnName = ui->tableValues->horizontalHeaderItem(column)->text().split(' ')[0];
		QString primary = "id";
		int primaryIndex = 0;

		if (driver == "QMYSQL")
		{
			QSqlQuery query(db);

			query.prepare("SELECT `" + columnName + "` FROM `" + tableName + "` LIMIT 1");
			query.exec();
			query.next();

			QVariant type = query.record().field(columnName).type();

			if (type.type() == type.DateTime)
			{
				QDateTimeEdit* edit = qobject_cast<QDateTimeEdit*>(ui->tableValues->cellWidget(row, column));

				query.prepare("SELECT `COLUMN_NAME`,`ORDINAL_POSITION` FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE `TABLE_SCHEMA`='" + db.databaseName() + "' AND `TABLE_NAME`='" + tableName + "' AND `CONSTRAINT_NAME`='PRIMARY';");
				query.exec();
				query.next();

				primary = query.value(0).toString();
				primaryIndex = query.value(1).toInt() - 1;

				QString columnName = ui->tableValues->horizontalHeaderItem(column)->text().split(' ')[0];
				QString primaryVal = ui->tableValues->item(row, primaryIndex)->text();

				query.prepare("UPDATE `" + tableName + "` SET `" + columnName + "`=? WHERE `" + primary + "`='" + primaryVal + "'");
				query.addBindValue(edit->dateTime().toString(Qt::ISODate));
				query.exec();
			}
		}
	}
}

ConnectionTab::~ConnectionTab()
{
	db = QSqlDatabase();
	QSqlDatabase::removeDatabase(connectionData->getName());

	delete ui;
}
