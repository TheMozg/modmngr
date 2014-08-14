#include "pluginmanager.h"

PluginManager::PluginManager(QObject *parent) :QObject(parent)
{
	m_model = new QStandardItemModel(this);
	m_ActiveFile = QDir::homePath().append("/AppData/Local/Skyrim/plugins.txt");
	m_OrderFile = QDir::homePath().append("/AppData/Local/Skyrim/loadorder.txt");
	m_dataDir.setNameFilters(QStringList() << "*.esm" << "*.esp");
}

bool PluginManager::setSkyrimPath(QString skyrimPath)
{
	if(QFile::exists(skyrimPath + "/TESV.exe") && m_dataDir.path() != skyrimPath+"/data")
	{
		m_dataDir.setPath(skyrimPath+"/data");
		return true;
	}
	else
	{
		return false;
	}
}

void PluginManager::checkSelection(QModelIndexList indexList)
{
	for(QModelIndex index: indexList)
	{
		m_model->setData(index, Qt::Checked, Qt::CheckStateRole);
	}
}

void PluginManager::unCheckSelection(QModelIndexList indexList)
{
	for(QModelIndex index: indexList)
	{
		m_model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
	}
}

QStringList PluginManager::readFile(QString filePath)
{
	QStringList list;
	QFile file(filePath);
	if (!file.open(QFile::ReadOnly | QFile::Text))
		return list;
	QTextStream ts(&file);
	while(!ts.atEnd())
		list.append(ts.readLine());
	file.close();
	list.removeAll("Skyrim.esm");
	list.removeAll("Update.esm");
	return list;
}

QStringList PluginManager::getPresent()
{
	m_dataDir.refresh();
	QStringList list = m_dataDir.entryList();
	list.removeAll("Skyrim.esm");
	list.removeAll("Update.esm");
	return list;
}

QStandardItem* PluginManager::createItem(QString text, Qt::CheckState state)
{
	QStandardItem *item = new QStandardItem();
	item->setText(text);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	item->setCheckState(state);
	return item;
}

void PluginManager::reload()
{
	qDebug() << "reloading plugins";
	m_model->clear();
	m_model->setHorizontalHeaderLabels(QStringList("Name"));
	QStringList sortedPlugins = readFile(m_OrderFile);
	QStringList activePlugins = readFile(m_ActiveFile);
	QStringList presentPlugins = getPresent();
	foreach(QString plugin, sortedPlugins)
	{
		if(presentPlugins.contains(plugin))
		{
			if(activePlugins.contains(plugin))
				m_model->appendRow(createItem(plugin, Qt::Checked));
			else
				m_model->appendRow(createItem(plugin, Qt::Unchecked));
		}
	}
	foreach(QString plugin, presentPlugins)
	{
		if(!sortedPlugins.contains(plugin))
		{
			m_model->appendRow(createItem(plugin, Qt::Unchecked));
		}
	}
}

void PluginManager::save()
{
	QStringList activeList;
	QStringList orderList;
	orderList.append("Skyrim.esm");
	orderList.append("Update.esm");
	activeList.append("Update.esm");
	for(int i = 0; i < m_model->rowCount(); i++)
	{
		orderList.append(m_model->item(i)->text());
		if (m_model->item(i)->checkState() == Qt::Checked)
			activeList.append(m_model->item(i)->text());
	}
	saveFile(m_ActiveFile, activeList);
	saveFile(m_OrderFile, orderList);
}

void PluginManager::saveFile(QString filePath, QStringList list)
{
	QFile file(filePath);
	QFileInfo info(filePath);
	m_dataDir.mkpath(info.path());
	file.open(QFile::WriteOnly | QFile::Text);
	QTextStream ts(&file);
	foreach(QString plugin, list)
		ts << plugin << endl;
	file.close();
}
