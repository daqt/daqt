#include "SavedConnections.hpp"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QTextStream>

QString SavedConnections::getPath()
{
#if defined(Q_OS_LINUX)
	QString config = qgetenv("XDG_CONFIG_DIRS").constData();

	if (config.isEmpty())
		return QDir::homePath() + "/.config/DAQt/connections";

	return config + "/DAQt/connections";
#elif defined(Q_OS_MAC)
	//TODO: Mac config
#elif defined(Q_OS_WIN32)
	//Todo: Win config
#endif
}

bool SavedConnections::addConnection(QString name, QMap<QString, QString> data)
{
	if (!QDir(SavedConnections::getPath() + QDir::separator() + name).exists())
		QDir().mkpath(SavedConnections::getPath() + QDir::separator() + name);
	else
	{
		QRegularExpression reg(".*\\ \\([0-9]\\)$");

		if (reg.match(name).hasMatch())
		{
			int newVal = QString(name[name.length() - 2]).toInt() + 1;
			name.remove(name.length() - 4, 4);

			name += QString(QString(" (") + QString::number(newVal) + QString(")"));
		}
		else
			name += " (1)";

		return SavedConnections::addConnection(name, data);
	}

	data.insert("name", name);

	QFile file(SavedConnections::getPath() + QDir::separator() + name + QDir::separator() + "con.fig");

	if (file.open(QIODevice::WriteOnly))
	{
		QTextStream stream(&file);

		for (QMap<QString, QString>::iterator it = data.begin(); it != data.end(); it++)
		{
			QString str = it.key() + ": " + it.value();
			int i = it.key().length() + it.value().length();

			for (int c = 0; c < str.length(); c++)
				stream << QChar(str.toUtf8()[c] ^ i);

			stream << "." << i << endl;
		}
	}
	else
		return false;

	file.close();

	return true;
}

bool SavedConnections::removeConnection(QString name)
{
	QDir connection = QDir(SavedConnections::getPath() + QDir::separator() + name);

	if (!connection.exists())
		return false;

	return connection.removeRecursively();
}

QMap<QString, QMap<QString, QString>> SavedConnections::getConnections()
{
	QStringList list = QDir(SavedConnections::getPath()).entryList(QDir::NoDotAndDotDot | QDir::AllDirs);

	QMap<QString, QMap<QString, QString>> databases;

	for (QStringList::iterator it = list.begin(); it != list.end(); it++)
	{
		QString dir = it.i->t();

		if (!QDir().exists(SavedConnections::getPath() + QDir::separator() + dir))
			continue;

		QMap<QString, QString> data = SavedConnections::getConnection(dir);

		databases.insert(it.i->t(), data);
	}

	return databases;
}

QMap<QString, QString> SavedConnections::getConnection(QString name)
{
	name = SavedConnections::getPath() + QDir::separator() + name;

	QFile file(name + QDir::separator() + "con.fig");

	QMap<QString, QString> data;

	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream stream(&file);

		while (!stream.atEnd())
		{
			QString str = stream.readLine();

			int i = QString(str.split(".")[1]).toInt();

			QString result;

			for (int c = 0; c < QString(str.split(".")[0]).length(); c++)
				result += QChar(str.toUtf8()[c] ^ i);

			data.insert(result.split(": ")[0], result.split(": ")[1]);
		}

		file.close();
	}

	return data;
}
