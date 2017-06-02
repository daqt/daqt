#include "Utils.hpp"

#include <QTcpSocket>
#include <QUrl>

bool Utils::pingUrl(QString url, int port)
{
	QTcpSocket* socket = new QTcpSocket();

	socket->connectToHost(QUrl(url).host(), port);

	if (socket->waitForConnected(100))
	{
		socket->disconnectFromHost();

		return true;
	}

	return false;
}
