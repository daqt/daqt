#include "Utils.hpp"

#include <QStandardPaths>

QDir Utils::getConfigDirectory()
{
	QDir config(QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation)[0]);

	if (!config.exists())
	{
		QDir().mkpath(config.absolutePath());
	}

	return config;
}

QString Utils::xorString(QString val, int key)
{
	QString str;

	for (int c = 0; c < val.length(); c++)
	{
		str += QChar(val.toUtf8()[c] ^ key);
	}

	return str;
}
