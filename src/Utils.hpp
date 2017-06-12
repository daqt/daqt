#ifndef UTILS_HPP
#define UTILS_HPP

#include <QDir>
#include <QString>

class Utils
{
public:
	static QDir getConfigDirectory();

	static QString xorString(QString val, int key);
};

#endif // UTILS_HPP
