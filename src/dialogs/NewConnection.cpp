#include "src/dialogs/NewConnection.hpp"
#include "ui_NewConnection.h"

#include <QAbstractSocket>
#include <QHostAddress>
#include <QSqlDatabase>
#include <QSqlError>

NewConnection::NewConnection(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::NewConnection)
{
	ui->setupUi(this);

	connect(ui->editHostname, SIGNAL(editingFinished()), this, SLOT(tryHost()));
	connect(ui->editPort, SIGNAL(editingFinished()), this, SLOT(tryHost()));

	connect(ui->editHostname, SIGNAL(textChanged(QString)), this, SLOT(resetHost(QString)));
	connect(ui->editPort, SIGNAL(textChanged(QString)), this, SLOT(resetHost(QString)));

	connect(ui->buttonConnect, SIGNAL(pressed()), this, SLOT(tryDatabase()));
	connect(ui->buttonCancel, SIGNAL(pressed()), this, SLOT(close()));
}

void NewConnection::tryHost()
{
	QAbstractSocket* socket = new QAbstractSocket(QAbstractSocket::TcpSocket, this);

	socket->connectToHost(QHostAddress(ui->editHostname->text()), ui->editPort->text().toInt());

	if (socket->waitForConnected(3000))
	{
		ui->editHostname->setStyleSheet("color: #00dd00;");
		ui->editPort->setStyleSheet("color: #00dd00;");
		socket->disconnectFromHost();
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

		this->close();
		//TODO: Save the data
	}
	else
	{
		QSqlError err = db.lastError();

		ui->textError->setStyleSheet("color: #ff0000;");
		ui->textError->setText(err.databaseText());

		db.close();
		db = QSqlDatabase();
	}

	QSqlDatabase::removeDatabase("testConnection");
}

NewConnection::~NewConnection()
{
	delete ui;
}
