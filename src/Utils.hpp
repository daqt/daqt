#ifndef UTILS_HPP
#define UTILS_HPP

#include <QString>

class Utils
{
public:
	static bool pingUrl(QString url, int port);

	static bool removeDir(QString dir);
};

#endif // UTILS_HPP
