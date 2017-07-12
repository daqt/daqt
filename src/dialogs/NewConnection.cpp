#include "src/dialogs/NewConnection.hpp"
#include "ui_NewConnection.h"

#include <QFileDialog>
#include <QMap>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>

#include "src/MainWindow.hpp"
#include "src/Utils.hpp"

NewConnection::NewConnection(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::NewConnection)
{
	ui->setupUi(this);

	connect(ui->editHostname, SIGNAL(textChanged(QString)), this, SLOT(resetHostColor(QString)));
	connect(ui->editPort, SIGNAL(textChanged(QString)), this, SLOT(resetHostColor(QString)));

	connect(ui->buttonConnect, SIGNAL(pressed()), this, SLOT(tryConnect()));
	connect(ui->buttonCancel, SIGNAL(pressed()), this, SLOT(close()));

	connect(ui->editName, SIGNAL(editingFinished()), this, SLOT(checkName()));
	connect(ui->editName, SIGNAL(textChanged(QString)), this, SLOT(resetNameColor(QString)));

	connect(ui->comboDriver, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeDriver(QString)));

	connect(ui->buttonBrowse, SIGNAL(clicked(bool)), this, SLOT(browseFile(bool)));

	connect(ui->editHostname, SIGNAL(textChanged(QString)), this, SLOT(checkFile(QString)));

	if (qobject_cast<MainWindow*>(parent) != NULL)
	{
		connect(this, SIGNAL(finished(int)), (MainWindow*)parent, SLOT(loadConnections(int)));
	}

	QStringList drivers = QSqlDatabase::drivers();

	if (drivers.length() > 0)
	{
		ui->comboDriver->removeItem(0);

		for (int i = 0; i < drivers.length(); i++)
		{
			if (drivers[i] == "QMYSQL")
			{
				ui->comboDriver->addItem("MySQL");
			}
			else if (drivers[i] == "QSQLITE")
			{
				ui->comboDriver->addItem("SQLite");
			}
			else if (drivers[i] == "QPSQL")
			{
				ui->comboDriver->addItem("PostgreSQL");
			}
		}
	}

	changeDriver(ui->comboDriver->currentText());
}

void NewConnection::setValues(Connection* connection)
{
	ui->editName->setText(connection->getName());

	if (connection->getDriver() == "QMYSQL" || connection->getDriver() == "QPSQL")
	{
		if (connection->getDriver() == "QMYSQL")
		{
			ui->comboDriver->setCurrentText("MySQL");
		}
		else if (connection->getDriver() == "QPSQL")
		{
			ui->comboDriver->setCurrentText("PostgreSQL");
		}

		ui->editHostname->setText(connection->getHost().host());
		ui->editPort->setText(QString::number(connection->getHost().port()));
		ui->editUsername->setText(connection->getUsername());

		if (connection->getPassword().isNull())
		{
			ui->checkPassword->setChecked(false);
		}
		else
		{
			ui->editPassword->setText(connection->getPassword());
		}
	}
	else if (connection->getDriver() == "QSQLITE")
	{
		ui->comboDriver->setCurrentText("SQLite");

		ui->editHostname->setText(connection->getPath());
	}

	this->setWindowTitle("Edit Connection");
	ui->buttonConnect->setText("Save");

	oldName = connection->getName();
}

void NewConnection::resetHostColor(QString)
{
	ui->editHostname->setStyleSheet("");
	ui->editPort->setStyleSheet("");
}

void NewConnection::resetNameColor(QString)
{
	ui->editName->setStyleSheet("");
}

void NewConnection::tryConnect()
{
	if (ui->comboDriver->currentText() == "SQLite")
	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "testConnection");
		db.setDatabaseName(ui->editHostname->text());

		if (db.open())
		{
			db = QSqlDatabase();
			QSqlDatabase::removeDatabase("testConnection");

			if (ui->editName->styleSheet() == "color: #ff0000;")
			{
				ui->textError->setStyleSheet("color: #ff0000;");
				ui->textError->setText("Invalid connection name");

				return;
			}

			Connection* connection = new Connection();
			connection->setName(ui->editName->text());
			connection->setDriver("QSQLITE");
			connection->setPath(ui->editHostname->text());

			if (!connection->isValid())
			{
				ui->textError->setStyleSheet("color: #ff0000;");
				ui->textError->setText("Some values are wrong");

				return;
			}

			connection->save();

			this->close();
		}
		else
		{
			QSqlError err = db.lastError();

			ui->textError->setStyleSheet("color: #ff0000;");
			ui->textError->setText(err.databaseText());

			db = QSqlDatabase();
			QSqlDatabase::removeDatabase("testConnection");
		}

		return;
	}

	if (ui->editHostname->text().isEmpty())
	{
		ui->editHostname->setText(ui->editHostname->placeholderText());
	}

	if (ui->editName->text().isEmpty())
	{
		ui->editName->setText(ui->editName->placeholderText());
		checkName();
	}

	QString driver = "QMYSQL";

	if (ui->comboDriver->currentText() == "MySQL")
	{
		driver = "QMYSQL";
	}
	else if (ui->comboDriver->currentText() == "PostgreSQL")
	{
		driver = "QPSQL";
	}

	QSqlDatabase db = QSqlDatabase::addDatabase(driver, "testConnection");
	db.setHostName(ui->editHostname->text());
	db.setPort(ui->editPort->text().toInt());
	db.setUserName(ui->editUsername->text());
	db.setPassword(ui->editPassword->text());

	if (db.open())
	{
		db.close();
		db = QSqlDatabase();
		QSqlDatabase::removeDatabase("testConnection");

		if (ui->editName->styleSheet() == "color: #ff0000;")
		{
			ui->textError->setStyleSheet("color: #ff0000;");
			ui->textError->setText("Invalid connection name");

			return;
		}

		Connection* connection = new Connection();
		connection->setName(ui->editName->text());
		connection->setHost(ui->editHostname->text(), ui->editPort->text().toInt());
		connection->setUsername(ui->editUsername->text());
		connection->setDriver(driver);

		if (ui->checkPassword->isChecked())
		{
			connection->setPassword(ui->editPassword->text());
		}

		if (!connection->isValid())
		{
			ui->textError->setStyleSheet("color: #ff0000;");
			ui->textError->setText("Some values are wrong");

			return;
		}

		connection->save();

		this->close();
	}
	else
	{
		QSqlError err = db.lastError();

		ui->textError->setStyleSheet("color: #ff0000;");
		ui->textError->setText(err.databaseText());

		db = QSqlDatabase();
		QSqlDatabase::removeDatabase("testConnection");
	}
}

void NewConnection::checkName()
{
	if (ui->editName->text().isEmpty())
	{
		ui->editName->setText(ui->editName->placeholderText());
	}

	QRegularExpression reg("^[a-zA-Z0-9\\_\\.\\-\\ ()]*$");
	QString name = ui->editName->text();

	if (reg.match(name).hasMatch())
	{
		QDir target(Utils::getConfigDirectory().absolutePath() + QDir::separator() + "connections" + QDir::separator() + name);

		while (target.exists() && oldName.isEmpty())
		{
			reg.setPattern(".*\\ \\([0-9]\\)$");

			if (reg.match(name).hasMatch())
			{
				int newVal = QString(name[name.length() - 2]).toInt() + 1;
				name.remove(name.length() - 4, 4);

				name += QString(QString(" (") + QString::number(newVal) + QString(")"));
			}
			else
			{
				name += " (1)";
			}

			target.setPath(Utils::getConfigDirectory().absolutePath() + QDir::separator() + "connections" + QDir::separator() + name);
		}

		ui->editName->setText(name);

		ui->editName->setStyleSheet("color: #00dd00;");
	}
	else
	{
		ui->editName->setStyleSheet("color: #ff0000;");
	}

	if (ui->editName->text().length() == 0)
	{
		resetNameColor(NULL);
	}
}

void NewConnection::checkFile(QString)
{
	if (ui->comboDriver->currentText() != "SQLite")
	{
		return;
	}

	if (QFile(ui->editHostname->text()).exists())
	{
		ui->editHostname->setStyleSheet("color: #00dd00;");
	}
	else
	{
		ui->editHostname->setStyleSheet("color: #ff0000;");
	}

	if (ui->editHostname->text().length() == 0)
	{
		resetHostColor(NULL);
	}
}

void NewConnection::changeDriver(QString driver)
{
	if (driver == "SQLite")
	{
		ui->editPassword->setDisabled(true);
		ui->editPort->setDisabled(true);
		ui->editUsername->setDisabled(true);
		ui->checkPassword->setDisabled(true);

		ui->buttonBrowse->setDisabled(false);

		ui->labelHostname->setText("Path: ");
		ui->editHostname->setPlaceholderText("database.db");
	}
	else
	{
		ui->editPassword->setDisabled(false);
		ui->editPort->setDisabled(false);
		ui->editUsername->setDisabled(false);
		ui->checkPassword->setDisabled(false);

		ui->buttonBrowse->setDisabled(true);

		ui->labelHostname->setText("Hostname: ");
		ui->editHostname->setPlaceholderText("127.0.0.1");

		if (driver == "MySQL")
		{
			ui->editPort->setPlaceholderText("3306");
			ui->editPort->setText("3306");
		}
		else if (driver == "PostgreSQL")
		{
			ui->editPort->setPlaceholderText("5432");
			ui->editPort->setText("5432");
		}
	}
}

void NewConnection::browseFile(bool)
{
	ui->editHostname->setText(QFileDialog::getOpenFileName(this, "Open database...", QDir().homePath(), "Database files (*.db *.sqlite);;All files (*)"));
}

NewConnection::~NewConnection()
{
	delete ui;
}
