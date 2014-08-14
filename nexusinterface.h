#ifndef NEXUSINTERFACE_H
#define NEXUSINTERFACE_H

#include <QtCore>
#include <QtNetwork>

struct NexusModInfo
{
	QString version;
	QString name;
	QHash<QString,QString> fileNameVersionHash;
};

class NexusInterface : public QObject
{
	Q_OBJECT
public:
	explicit NexusInterface(QObject *parent = 0);

	void requestModInfoList(QStringList nexusIDList);
	void requestModFileList(QString nexusID);
	void requestInfoForMods(QStringList nexusIDList);

signals:
	void receivedFileList(QString nexusID);
	void receivedVersion(QString nexusID);
	void recievedAllModInfo(QHash<QString, NexusModInfo>);

private slots:
	void receivedReply(QNetworkReply *reply);

public slots:

private:
	int count;
	QTime t;
	QNetworkAccessManager *networkAccessManager;
	QHash<QString, NexusModInfo> nexusModInfoIDHash;

};

#endif // NEXUSINTERFACE_H
