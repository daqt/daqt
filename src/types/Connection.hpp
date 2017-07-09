#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <QString>
#include <QUrl>

class Connection
{
public:
	Connection();
	Connection(QString name);
	Connection(QString name, QUrl host, QString username, QString password, QString driver);
	Connection(QString name, QString hostname, int port, QString username, QString password, QString driver);

	QString getName();
	void setName(QString name);

	QUrl getHost();
	void setHost(QUrl host);
	void setHost(QString hostname, int port);

	QString getUsername();
	void setUsername(QString username);

	QString getPassword();
	void setPassword(QString password);

	QString getDriver();
	void setDriver(QString driver);

	QString getPath();
	void setPath(QString path);

	bool isValid();

	bool load();
	bool save();

private:
	QString name;
	QUrl host;
	QString username;
	QString password;
	QString driver;
	QString path;
};

#endif // CONNECTION_HPP
