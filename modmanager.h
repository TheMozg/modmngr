#ifndef MODMANAGER_H
#define MODMANAGER_H

#include "modInfo.h"
#include "nexusinterface.h"
#include "archiver.h"
#include "sortfilterproxymodel.h"
#include <QtWidgets>

class ModManager : public QObject
{
	Q_OBJECT

public:
	explicit ModManager(QObject *parent = 0);
	bool m_suModeIsEnabled;
	void reload();
	void save(QString filePath);
	void saveDataCash(QString filePath);
	void setPaths(QString skyrimPath, QString modsPath);
	void accessNexusDatabase();
	void checkSelection(QModelIndexList indexList);
	void unCheckSelection(QModelIndexList indexList);

	SortFilterProxyModel* modFilter;
	SortFilterProxyModel* fileFilter;

private:
	void refreshOnDrop();
	void syncModOrder();
	void applyChanges();
	void exportToModel();
	void syncSelection(QModelIndex index);
	QList<ModInfo> m_modList;
	QDir *modsDir;
	QDir *dataDir;
	QStandardItemModel *m_model;
	void refreshOnCheck(const QModelIndex & topLeft, const QModelIndex & bottomRight);
	int indexFromArchiveName(QString archiveName);
	int indexFromFileName(QString fileName, ModInfo modInfo);
	NexusInterface *nexusInterface;
	void reset();
	void loadDataCash(QString filePath);
	QList<ModInfo> loadModInfo(QString filePath);
	void replyFinished(QHash<QString, NexusModInfo> nexusModInfoIDHash);
	QStringList m_sortOrder;
	QMap<QString, QString> m_dataMap;
	QList<QStandardItem *> createFileItems(FileInfo fileInfo);
	QList<QStandardItem *> createSubmodItems(SubmodInfo submodInfo);
	QList<QStandardItem *> createModItems(ModInfo modInfo);
	QString calculateCheckSum(QString filePath);
	bool fileMatched(FileInfo fileInfo);
	bool layoutChanged;
signals:
	void fileLoaded(QString modName);
	void filesCounted(int installCount);
	void filesCopied(QString modName,int chunk);
	void finishedExporting();
};

#endif // MODMANAGER_H
