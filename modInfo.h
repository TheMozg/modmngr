#ifndef MODINFO_H
#define MODINFO_H

#include <QtCore>

struct ModInfoNS
{
	static QPair<QString,QString> getNexusIDAndVersion(QString fileName)
	{
		fileName.remove(QRegularExpression(".*[_ ]"));
		QRegularExpression regex("-(\\d+)[-.]");
		QRegularExpressionMatch firstMatch = regex.match(fileName);
		if(!firstMatch.hasMatch())
			return qMakePair(QString(), QString());
		QRegularExpressionMatch maxMatch;
		QRegularExpressionMatch match = firstMatch;
		while(match.hasMatch())
		{
			if(match.captured(1).toInt() > maxMatch.captured(1).toInt())
			{
				maxMatch = match;
			}
			int offset = match.capturedEnd(1);
			match = regex.match(fileName, offset);
		}
		if(maxMatch.captured(1).count() - firstMatch.captured(1).count() >= 1) //works with 99.9 % accuracy
		{
			match = maxMatch;
		}
		else
		{
			match = firstMatch;
		}
		fileName = fileName.mid(match.capturedEnd(1)+1);
		fileName.truncate(fileName.indexOf('.'));
		fileName.replace('-', '.');
		return qMakePair(match.captured(1), fileName);
	}
};

struct FileInfo
{
	QString name;
	QString path;
	QString checkSum;
};

struct SubmodInfo
{
	QString name;
	bool isChecked;
	QList<FileInfo> fileList;
};

struct ModInfo
{
	QString name;
	QString nexusID;
	QString version;
	QString latestVersion;
	QList<SubmodInfo> submodList;
	QList<FileInfo> skipList;
	//bool isExpanded;
};

struct ModGroupInfo
{
	QString name;
	QString version;
	QList<ModInfo> modList;
};


#endif // MODINFO_H
