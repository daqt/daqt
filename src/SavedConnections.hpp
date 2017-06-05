#ifndef SAVEDCONNECTIONS_HPP
#define SAVEDCONNECTIONS_HPP

#include <QMap>
#include <QObject>
#include <QString>

class SavedConnections
{
public:
	static QString getPath();

	static bool addConnection(QString name, QMap<QString, QString> data);
	static bool removeConnection(QString name);

	static QMap<QString, QMap<QString, QString>> getConnections();
	static QMap<QString, QString> getConnection(QString name);
};

#endif // SAVEDCONNECTIONS_HPP
