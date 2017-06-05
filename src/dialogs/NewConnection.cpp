#include "src/dialogs/NewConnection.hpp"
#include "ui_NewConnection.h"

#include <QMap>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>

#include "src/MainWindow.hpp"
#include "src/SavedConnections.hpp"
#include "src/Utils.hpp"

NewConnection::NewConnection(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::NewConnection)
{
	ui->setupUi(this);

	connect(ui->editHostname, SIGNAL(textChanged(QString)), this, SLOT(resetHost(QString)));
	connect(ui->editPort, SIGNAL(textChanged(QString)), this, SLOT(resetHost(QString)));

	connect(ui->buttonConnect, SIGNAL(pressed()), this, SLOT(tryDatabase()));
	connect(ui->buttonCancel, SIGNAL(pressed()), this, SLOT(close()));

	connect(ui->editName, SIGNAL(editingFinished()), this, SLOT(tryName()));
	connect(ui->editName, SIGNAL(textChanged(QString)), this, SLOT(resetName(QString)));

	connect(this, SIGNAL(finished(int)), (MainWindow*)parent, SLOT(loadDatabases(int)));
}

void NewConnection::setValues(QMap<QString, QString> data)
{
	oldName = data["name"];

	ui->editName->setText(oldName);
	ui->editHostname->setText(data["hostname"]);
	ui->editPort->setText(data["port"]);
	ui->editUsername->setText(data["username"]);
	ui->editPassword->setText(data["password"]);

	if (data["type"] == "MySQL")
		ui->comboType->setCurrentIndex(0);

	ui->buttonConnect->setText("Save");
}

void NewConnection::tryHost()
{
	QString url = ui->editHostname->text();

	bool connected = false;

	if (!(url.startsWith("http://") && url.startsWith("https://")))
		connected = (Utils::pingUrl("http://" + url, ui->editPort->text().toInt()) | Utils::pingUrl("https://" + url, ui->editPort->text().toInt()));
	else
		connected = Utils::pingUrl(url, ui->editPort->text().toInt());

	if (connected)
	{
		ui->editHostname->setStyleSheet("color: #00dd00;");
		ui->editPort->setStyleSheet("color: #00dd00;");
	}
	else
	{
		ui->editHostname->setStyleSheet("color: #ff0000;");
		ui->editPort->setStyleSheet("color: #ff0000;");
	}
}

void NewConnection::resetHost(QString)
{
	ui->editHostname->setStyleSheet("");
	ui->editPort->setStyleSheet("");
}

void NewConnection::tryDatabase()
{
	tryHost();

	QString driver = "QMYSQL";

	switch (ui->comboType->currentIndex())
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

		QMap<QString, QString> map;
		map.insert("type", ui->comboType->currentText());
		map.insert("hostname", ui->editHostname->text());
		map.insert("port", ui->editPort->text());
		map.insert("username", ui->editUsername->text());

		if (ui->checkPassword->isChecked())
			map.insert("password", ui->editPassword->text()); //TODO: do something for safety

		if (!oldName.isEmpty())
			SavedConnections::removeConnection(oldName);

		if (ui->editName->text().length() == 0)
			SavedConnections::addConnection(ui->editName->placeholderText(), map);
		else
			SavedConnections::addConnection(ui->editName->text(), map);

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

void NewConnection::tryName()
{
	QRegularExpression reg("^[a-zA-Z0-9\\_\\.\\-\\ ()]*$");

	if (reg.match(ui->editName->text()).hasMatch())
		ui->editName->setStyleSheet("color: #00dd00;");
	else
		ui->editName->setStyleSheet("color: #ff0000;");

	if (ui->editName->text().length() == 0)
		resetName("");
}

void NewConnection::resetName(QString)
{
	ui->editName->setStyleSheet("");
}

NewConnection::~NewConnection()
{
	delete ui;
}
