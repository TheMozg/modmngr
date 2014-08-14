#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <QtCore>
#include <modInfo.h>

class Archive: public QObject
{
	Q_OBJECT
private:
	QString exePath;
	QString path;
	QTimer *timer;
	quint64 total, prcount;
	QSignalMapper *signalMapper;
	static QString relativePath(QString filePath);
	static QString subDir(QString filePath, QString relativeFilePath);
public:
	static QList<ModInfo> list(QStringList mylist);
	QProcess *process;
	Archive(QString pathToZip);
	void extract(QString outputPath, QList<FileInfo> fileList);
	static QString getFileCRC(QString fileName);
	static QList<ModInfo> extractIdAndVersion(QString outputPath, QList<ModInfo> modInfoList, QDir dir);
public slots:
	void update(QString outputPath);
signals:
	void filesExtracted(QString name, int extracted);
	void extractionFinished();

};

#endif // ARCHIVER_H
