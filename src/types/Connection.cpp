#include "src/types/Connection.hpp"

#include <QDir>
#include <QFile>
#include <QTextStream>

#include "src/Utils.hpp"

Connection::Connection()
{

}

Connection::Connection(QString name)
{
	this->setName(name);
}

Connection::Connection(QString name, QUrl host, QString username, QString password, QString driver)
{
	this->setName(name);
	this->setHost(host);
	this->setUsername(username);
	this->setPassword(password);
	this->setDriver(driver);
}

Connection::Connection(QString name, QString hostname, int port, QString username, QString password, QString driver)
{
	this->setName(name);
	this->setHost(hostname, port);
	this->setUsername(username);
	this->setPassword(password);
	this->setDriver(driver);
}

QString Connection::getName()
{
	return this->name;
}

void Connection::setName(QString name)
{
	this->name = name;
}

QUrl Connection::getHost()
{
	return this->host;
}

void Connection::setHost(QUrl host)
{
	this->host = host;
}

void Connection::setHost(QString hostname, int port)
{
	QUrl host;

	host.setHost(hostname);
	host.setPort(port);

	setHost(host);
}

QString Connection::getUsername()
{
	return this->username;
}

void Connection::setUsername(QString username)
{
	this->username = username;
}

QString Connection::getPassword()
{
	return this->password;
}

void Connection::setPassword(QString password)
{
	this->password = password;
}

QString Connection::getDriver()
{
	return this->driver;
}

void Connection::setDriver(QString driver)
{
	this->driver = driver;
}

QString Connection::getPath()
{
	return this->path;
}

void Connection::setPath(QString path)
{
	this->path = path;
}

bool Connection::isValid()
{
	if (!(this->getName().isEmpty() && this->getHost().isEmpty() && this->getUsername().isEmpty() && this->getDriver().isEmpty()))
	{
		return true;
	}

	if (this->getDriver() == "QSQLITE" && !(this->getName().isEmpty() && this->getPath().isEmpty()))
	{
		return true;
	}

	return false;
}

bool Connection::load()
{
	if (this->getName().isEmpty())
	{
		return false;
	}

	QFile file(Utils::getConfigDirectory().absolutePath() + QDir::separator() + "connections" + QDir::separator() + this->getName() + QDir::separator() + "con.fig");

	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream stream(&file);

		while (!stream.atEnd())
		{
			QString line = stream.readLine();
			int x = QString(line.split(".")[1]).toInt();
			QString result = Utils::xorString(line.split(".")[0], x);

			QString key = result.split(": ")[0];
			QString val = result.split(": ")[1];

			if (key == "host")
			{
				setHost(QUrl(val));
			}
			if (key == "username")
			{
				setUsername(val);
			}
			if (key == "password")
			{
				setPassword(val);
			}
			if (key == "driver")
			{
				setDriver(val);
			}
			if (key == "path")
			{
				setPath(val);
			}
		}

		file.close();
	}
	else
	{
		return false;
	}

	return true;
}

bool Connection::save()
{
	if (!this->isValid())
	{
		return false;
	}

	QDir().mkpath(Utils::getConfigDirectory().absolutePath() + QDir::separator() + "connections" + QDir::separator() + this->getName());
	QFile file(Utils::getConfigDirectory().absolutePath() + QDir::separator() + "connections" + QDir::separator() + this->getName() + QDir::separator() + "con.fig");

	if (file.exists())
		file.remove();

	if (file.open(QIODevice::WriteOnly))
	{
		QTextStream stream(&file);

		stream << Utils::xorString("driver: " + this->getDriver(), this->getDriver().length()) << "." << this->getDriver().length() << endl;

		if (this->getDriver() == "QSQLITE")
		{
			stream << Utils::xorString("path: " + this->getPath(), this->getPath().length()) << "." << this->getPath().length() << endl;
		}
		else
		{
			stream << Utils::xorString("host: " + this->getHost().toString(), this->getHost().toString().length()) << "." << this->getHost().toString().length() << endl;
			stream << Utils::xorString("username: " + this->getUsername(), this->getUsername().length()) << "." << this->getUsername().length() << endl;

			if (!this->getPassword().isNull())
			{
				stream << Utils::xorString("password: " + this->getPassword(), this->getPassword().length()) << "." << this->getPassword().length() << endl;
			}
		}

		file.close();
	}
	else
	{
		return false;
	}

	return true;
}
