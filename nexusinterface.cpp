#include "nexusinterface.h"

NexusInterface::NexusInterface(QObject *parent) : QObject(parent)
{
	networkAccessManager = new QNetworkAccessManager(this);
	connect(networkAccessManager, &QNetworkAccessManager::finished, this, &NexusInterface::receivedReply);
}

void NexusInterface::requestModInfoList(QStringList nexusIDList)
{
	QNetworkRequest request(QUrl("http://nmm.nexusmods.com/skyrim/Mods/GetUpdates?ModList=["+nexusIDList.join(',')+"]&game_id=110"));
	request.setHeader(QNetworkRequest::UserAgentHeader, "Nexus Client v0.49.6");
	request.setAttribute(QNetworkRequest::User, "mil");
	networkAccessManager->post(request, QByteArray());
	//qDebug() << request.url().toString();
}

void NexusInterface::requestModFileList(QString nexusID)
{
	QNetworkRequest request(QUrl("http://nmm.nexusmods.com/skyrim/files/indexfrommod/"+nexusID+"/?game_id=110"));
	request.setHeader(QNetworkRequest::UserAgentHeader, "Nexus Client v0.49.6");
	request.setAttribute(QNetworkRequest::User, "mfl");
	networkAccessManager->get(request);
}

void NexusInterface::requestInfoForMods(QStringList nexusIDList)
{
	t = QTime(0,0);
	t.start();
	nexusIDList.removeAll("");
	nexusIDList.removeDuplicates();
	count = nexusIDList.count();
	nexusIDList.sort();
	requestModInfoList(nexusIDList);
	//qDebug() << "sent";
	//qDebug() << nexusIDList;
	//for(QString nexusID: nexusIDList)
		//requestModFileList(nexusID);
	//qDebug() << nexusIDList;
}

void NexusInterface::receivedReply(QNetworkReply *reply)
{
	QByteArray data = reply->readAll();
	//qDebug() << data.count();
	QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
	if(reply->request().attribute(QNetworkRequest::User).toString() == "mil")
	{
		QJsonArray array = jsonDocument.array();
		for(int i = 0; i < array.count(); i++)
		{
			QJsonObject object = array.at(i).toObject();
			QString name = object.value("name").toString();
			QString nexusID = QString::number(object.value("id").toDouble());
			QString version = object.value("version").toString();
			//qDebug() << nexusID << name << version;
			if(!nexusModInfoIDHash.contains(nexusID))
				nexusModInfoIDHash.insert(nexusID, NexusModInfo());
			NexusModInfo &nexusModInfo = nexusModInfoIDHash[nexusID];
			nexusModInfo.version = version;
			nexusModInfo.name = name;
		}
	}
	if(reply->request().attribute(QNetworkRequest::User).toString() == "mfl")
	{
		QJsonArray array = jsonDocument.array();
		for(int i = 0; i < array.count(); i++)
		{
			QJsonObject object = array.at(i).toObject();
			QString name = object.value("name").toString();
			QString nexusID = QString::number(object.value("mod_id").toDouble());
			QString version = object.value("version").toString();
			//qDebug() << nexusID << name << version;
			if(!nexusModInfoIDHash.contains(nexusID))
				nexusModInfoIDHash.insert(nexusID, NexusModInfo());
			NexusModInfo &nexusModInfo = nexusModInfoIDHash[nexusID];
			nexusModInfo.fileNameVersionHash.insert(name, version);
		}
	}
	//qDebug() << "received";
	//qDebug() << nexusModInfoIDHash.keys()<< nexusModInfoIDHash.count() << count;
	//if(nexusModInfoIDHash.count() == count)  TEMPORAL FIX WONT WORK ON MORE THAN ONE REQUEST
	//{
		//qDebug() << t.elapsed();
		emit recievedAllModInfo(nexusModInfoIDHash);
	//}
}
