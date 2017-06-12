#include "src/dialogs/NewConnection.hpp"
#include "ui_NewConnection.h"

#include <QMap>
#include <QRegularExpression>
#include <QSqlDatabase>
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

	if (qobject_cast<MainWindow*>(parent) != NULL)
	{
		connect(this, SIGNAL(finished(int)), (MainWindow*)parent, SLOT(loadConnections(int)));
	}
}

void NewConnection::setValues(Connection* connection)
{
	ui->editName->setText(connection->getName());
	ui->editHostname->setText(connection->getHost().host());
	ui->editPort->setText(QString::number(connection->getHost().port()));
	ui->editUsername->setText(connection->getUsername());

	if (connection->getPassword().isEmpty())
	{
		ui->checkPassword->setChecked(false);
	}
	else
	{
		ui->editPassword->setText(connection->getPassword());
	}

	if (connection->getDriver() == "QMYSQL")
	{
		ui->comboDriver->setCurrentIndex(0);
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

	switch (ui->comboDriver->currentIndex())
	{
	case 0:
		driver = "QMYSQL";
		break;
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

		if (ui->comboDriver->currentText() == "MySQL")
		{
			connection->setDriver("QMYSQL");
		}

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

		db.close();
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

NewConnection::~NewConnection()
{
	delete ui;
}
