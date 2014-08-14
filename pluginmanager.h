#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtWidgets>

class PluginManager : public QObject
{
	Q_OBJECT

public:
	explicit PluginManager(QObject *parent = 0);
	bool setSkyrimPath(QString skyrimPath);
	QStandardItemModel *m_model;
	void checkSelection(QModelIndexList indexList);
	void unCheckSelection(QModelIndexList indexList);

public slots:
	void save();
	void reload();

private:
	QDir m_dataDir;
	QString m_ActiveFile;
	QString m_OrderFile;
	QStringList readFile(QString filePath);
	void saveFile(QString filePath, QStringList list);
	QStringList getPresent();
	QStandardItem* createItem(QString text, Qt::CheckState state);
};

#endif // PLUGINMANAGER_H
