#ifndef UTILS_HPP
#define UTILS_HPP

#include <QDir>
#include <QList>
#include <QString>
#include <QVariant>

class Utils
{
public:
	static QDir getConfigDirectory();

	static QString xorString(QString val, int key);

	template<typename T>
	static inline int findInList(QList<T> list, T item)
	{
		for (int i = 0; i < list.length(); i++)
		{
			if (QVariant().fromValue(list.at(i)).toString() == QVariant().fromValue(item).toString())
			{
				return i;
			}
		}

		for (int i = 0; i < list.length(); i++)
		{
			if (QVariant().fromValue(list.at(i)).toString().startsWith(QVariant().fromValue(item).toString()))
			{
				return i;
			}
		}

		return -1;
	}
};

#endif // UTILS_HPP
