#include "modmanager.h"

ModManager::ModManager(QObject *parent) :QObject(parent)
{
	dataDir = new QDir();
	modsDir = new QDir();
	modsDir->setNameFilters(QStringList() << "*.7z" << "*.zip");
	modsDir->setFilter(QDir::Files);
	nexusInterface = new NexusInterface(this);
	m_model = new QStandardItemModel(this);
	m_model->setHorizontalHeaderLabels(QStringList() << "Name"<< "Path"<< "Checksum"<< "Subdir"<< "Latest Version" << "Version");
	fileFilter = new SortFilterProxyModel(false);
	modFilter = new SortFilterProxyModel(true);
	fileFilter->setSourceModel(m_model);
	modFilter->setSourceModel(m_model);
	connect(nexusInterface, &NexusInterface::recievedAllModInfo, this, &ModManager::replyFinished);
	connect(modFilter, &SortFilterProxyModel::rowsInserted, this, &ModManager::refreshOnDrop, Qt::QueuedConnection);
	connect(modFilter, &SortFilterProxyModel::dataChanged, this, &ModManager::refreshOnCheck);
}

void ModManager::setPaths(QString skyrimPath, QString modsPath)
{
	QString oldSP = dataDir->path();
	QString oldMP = modsDir->path();
	if(QFile::exists(skyrimPath + "/TESV.exe") && QFile::exists(skyrimPath + "/data"))
		dataDir->setPath(skyrimPath+"/data");
	if(QFile::exists(modsPath))
		modsDir->setPath(modsPath);
	if(oldMP != modsDir->path())
		reload();
	if(oldSP != dataDir->path() && oldMP == modsDir->path())
		refreshOnCheck(QModelIndex(), QModelIndex());
}

void ModManager::accessNexusDatabase()
{
	QStringList idlist;
	for(ModInfo modInfo: m_modList)
	{
		idlist.append(modInfo.nexusID);
	}
	nexusInterface->requestInfoForMods(idlist);
	//qDebug() << idlist;
}

void ModManager::checkSelection(QModelIndexList indexList)
{
	disconnect(modFilter, &SortFilterProxyModel::dataChanged, this, &ModManager::refreshOnCheck);
	for(QModelIndex index: indexList)
	{
		modFilter->setData(index, Qt::Checked, Qt::CheckStateRole);
		syncSelection(index);
	}
	connect(modFilter, &SortFilterProxyModel::dataChanged, this, &ModManager::refreshOnCheck);
	refreshOnCheck(QModelIndex(), QModelIndex());
}

void ModManager::unCheckSelection(QModelIndexList indexList)
{
	disconnect(modFilter, &SortFilterProxyModel::dataChanged, this, &ModManager::refreshOnCheck);
	for(QModelIndex index: indexList)
	{
		modFilter->setData(index, Qt::Unchecked, Qt::CheckStateRole);
		syncSelection(index);
	}
	connect(modFilter, &SortFilterProxyModel::dataChanged, this, &ModManager::refreshOnCheck);
	refreshOnCheck(QModelIndex(), QModelIndex());
}

void ModManager::refreshOnDrop()
{
	qDebug() << "refreshOnDrop";
	syncModOrder();

	disconnect(modFilter, &SortFilterProxyModel::rowsInserted, this, &ModManager::refreshOnDrop);
	applyChanges();
	exportToModel();
	connect(modFilter, &SortFilterProxyModel::rowsInserted, this, &ModManager::refreshOnDrop, Qt::QueuedConnection);
}

void ModManager::reload()
{
	disconnect(modFilter, &SortFilterProxyModel::dataChanged, this, &ModManager::refreshOnCheck);
	disconnect(modFilter, &SortFilterProxyModel::rowsInserted, this, &ModManager::refreshOnDrop);
	reset();
	loadDataCash(QDir::current().filePath("datacash.txt"));
	QTime t(0,0);
	t.start();

	QStringList modList;
	for(QString entry: modsDir->entryList())
	{
		modList.append(modsDir->filePath(entry));
	}
	QList<ModInfo> modInfoList = Archive::list(modList);
	QList<ModInfo> list = loadModInfo("modlist.txt");
	for(int i = 0; i < list.count(); i++)
	{
		for(int j = 0; j < modInfoList.count(); j++)
		{
			if(list.at(i).name == modInfoList.at(j).name)
			{
				QMutableListIterator<SubmodInfo> it(modInfoList[j].submodList);
				while(it.hasNext())
				{
					SubmodInfo &submodInfo = it.next();
					submodInfo.isChecked = (indexFromFileName(submodInfo.name, list.at(i)) != -1);
				}
				modInfoList[j].latestVersion = list.at(i).latestVersion;
				modInfoList.move(j, i);
				break;
			}
		}
	}
	//Archive::extractIdAndVersion(QDir::currentPath()+"/temp", modInfoList, *modsDir);
	for(ModInfo modInfo: modInfoList)
	{
//		modInfo.isExpanded = false;
		QPair<QString,QString> idVerPair = ModInfoNS::getNexusIDAndVersion(modInfo.name);
		modInfo.nexusID = idVerPair.first;
		modInfo.version = idVerPair.second;

		m_modList.append(modInfo);
	}
	m_modGroupList = groupById(m_modList);
	qDebug() <<"loading time" << t.elapsed();
	connect(modFilter, &SortFilterProxyModel::dataChanged, this, &ModManager::refreshOnCheck);
	connect(modFilter, &SortFilterProxyModel::rowsInserted, this, &ModManager::refreshOnDrop, Qt::QueuedConnection);
	refreshOnCheck(QModelIndex(), QModelIndex());
}

QList<QStandardItem *> ModManager::createFileItems(FileInfo fileInfo)
{
	QStringList textList;
	textList << fileInfo.name.section("/",-1) << fileInfo.path << fileInfo.checkSum << "";
	QList<QStandardItem *> list;
	for(QString text: textList)
	{
		QStandardItem *item = new QStandardItem(text);
		item->setFlags(Qt::ItemIsEnabled);
		item->setData("fileOrGroup");
		list.append(item);
	}
	return list;
}

QList<QStandardItem *> ModManager::createSubmodItems(SubmodInfo submodInfo)
{
	QStringList textList;
	textList << submodInfo.name << "" << "" << "" << "" << "";
	QList<QStandardItem *> list;
	for(QString text: textList)
	{
		QStandardItem *item = new QStandardItem(text);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren);
		list.append(item);
	}
	return list;
}

QList<QStandardItem *> ModManager::createModItems(ModInfo modInfo)
{
	QList<QStandardItem *> list;
	QStandardItem *modItem = new QStandardItem(modInfo.name);
	modItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsTristate | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	modItem->setCheckState(Qt::Unchecked);
	modItem->setColumnCount(6);
	list.append(modItem);
	QStringList textList;
	textList << "" << "" << "" << modInfo.latestVersion << modInfo.version;
	for(QString text: textList)
	{
		QStandardItem *item = new QStandardItem(text);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
		list.append(item);
	}
	return list;
}

QList<QStandardItem *> ModManager::createModGroupItems(ModGroupInfo modGroupInfo)
{
	QList<QStandardItem *> list;
	QStandardItem *item = new QStandardItem(modGroupInfo.name);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsTristate | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	item->setCheckState(Qt::Unchecked);
	item->setColumnCount(6);
	list.append(item);
	QStringList textList;
	textList << "" << "" << "" << modGroupInfo.latestVersion << modGroupInfo.version;
	for(QString text: textList)
	{
		QStandardItem *item = new QStandardItem(text);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
		list.append(item);
	}
	return list;
}

void ModManager::reset()
{
	m_model->removeRows(0, m_model->rowCount());
	m_modList.clear();
	m_sortOrder.clear();
	//m_dataMap.clear();
}

QString ModManager::calculateCheckSum(QString filePath)
{
	if(!QFile::exists(filePath))
		return QString();
	return Archive::getFileCRC(filePath);
}


bool ModManager::fileMatched(FileInfo fileInfo)
{
	QString dataFilePath = dataDir->filePath(fileInfo.name);
	if(!m_dataMap.contains(dataFilePath))
		m_dataMap.insert(dataFilePath, calculateCheckSum(dataFilePath));
	QString fileHash = m_dataMap.value(dataFilePath);
	if(fileHash == fileInfo.checkSum)
		return true;
	else
		return false;
}

void ModManager::save(QString filePath)
{
	QJsonArray jsonArray;
	for(ModInfo modInfo: m_modList)
	{
		QJsonObject jsonObject;
		jsonObject.insert("Name", modInfo.name);
		jsonObject.insert("Latest Version", modInfo.latestVersion);
		QJsonArray jsonCheckedSubmods;
		for(SubmodInfo submodInfo: modInfo.submodList)
			if(submodInfo.isChecked)
				jsonCheckedSubmods.append(submodInfo.name);
		jsonObject.insert("CheckedSubmods", jsonCheckedSubmods);
		jsonArray.append(jsonObject);
	}
	QJsonDocument jsonDoc(jsonArray);
	QFile file(filePath);
	file.open(QFile::WriteOnly | QFile::Text);
	file.write(jsonDoc.toJson());
	file.close();
}

void ModManager::saveDataCash(QString filePath)
{
	QJsonArray jsonArray;
	QMapIterator<QString, QString> i(m_dataMap);
	while (i.hasNext())
	{
		i.next();
		QJsonObject jsonObject;
		jsonObject.insert("Path", i.key());
		QFileInfo fileInfo(i.key());
		jsonObject.insert("Date", fileInfo.lastModified().toString());
		jsonObject.insert("CheckSum", i.value());
		jsonArray.append(jsonObject);
	}
	QJsonDocument jsonDoc(jsonArray);
	QFile file(filePath);
	file.open(QFile::WriteOnly | QFile::Text);
	file.write(jsonDoc.toJson());
	file.close();
}

void ModManager::loadDataCash(QString filePath)
{
	QFile file(filePath);
	file.open(QFile::ReadOnly | QFile::Text);
	QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
	file.close();
	QJsonArray jsonArray = jsonDoc.array();
	for(int i = 0; i < jsonArray.count(); i++)
	{
		QJsonObject jsonObject = jsonArray.at(i).toObject();
		QString path = jsonObject.find("Path").value().toString();
		QString date = jsonObject.find("Date").value().toString();
		QString checksum = jsonObject.find("CheckSum").value().toString();
		QFileInfo fileInfo(path);
		if(date == fileInfo.lastModified().toString())
			m_dataMap.insert(path, checksum);
	}
}

QList<ModInfo> ModManager::loadModInfo(QString filePath)
{
	QFile file(filePath);
	file.open(QFile::ReadOnly | QFile::Text);
	QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
	file.close();
	QList<ModInfo> modInfoList;
	QJsonArray jsonArray = jsonDoc.array();
	for(int i = 0; i < jsonArray.count(); i++)
	{
		QJsonObject jsonObject = jsonArray.at(i).toObject();
		ModInfo modInfo;
		modInfo.name = jsonObject.find("Name").value().toString();
		modInfo.latestVersion = jsonObject.find("Latest Version").value().toString();
		QJsonArray jsonCheckedSubmods = jsonObject.find("CheckedSubmods").value().toArray();
		for(int i = 0; i < jsonCheckedSubmods.count(); i++)
		{
			SubmodInfo submodInfo;
			submodInfo.name = jsonCheckedSubmods.at(i).toString();
			modInfo.submodList.append(submodInfo);
		}
		modInfoList.append(modInfo);
	}
	return modInfoList;
}

void ModManager::syncModOrder()
{
	for(int modGroupRow = 0; modGroupRow < modFilter->rowCount(); modGroupRow++)
	{
		QString name;
		for(int col = 0; col < modFilter->columnCount(); col++)
		{
			if(!modFilter->index(modGroupRow,col).data().toString().isEmpty())
			{
				name = modFilter->index(modGroupRow,col).data().toString();
				break;
			}
		}
		int oldModGroupRow = indexFromModGroupName(name);
		if(oldModGroupRow != modGroupRow && oldModGroupRow != -1)
		{
			ModGroupInfo modGroupInfo = m_modGroupList.takeAt(oldModGroupRow);
			m_modGroupList.insert(modGroupRow, modGroupInfo);
		}
	}
}

QList<ModGroupInfo> ModManager::groupById(QList<ModInfo> modInfoList)
{
	QHash<QString, ModGroupInfo> hash;
	for(ModInfo modInfo: modInfoList)
	{
		if(hash.contains(modInfo.nexusID))
		{
			hash[modInfo.nexusID].version = qMax(hash[modInfo.nexusID].version, modInfo.version);
			hash[modInfo.nexusID].modList.append(modInfo);
		}
		else
		{
			ModGroupInfo modGroupInfo;
			modGroupInfo.name = modInfo.name;
			modGroupInfo.version = modInfo.version;
			modGroupInfo.modList.append(modInfo);
			hash.insert(modInfo.nexusID, modGroupInfo);
		}
	}
	return hash.values();
}

void ModManager::applyChanges()
{
	QTime t(0,0);
	t.start();
	qDebug() << "apply changes";
	QStringList filenames;
	QStringList tempList;
	QMultiHash<QString, FileInfo> installMap;
	QListIterator<ModInfo> modIter(m_modList);
	modIter.toBack();
	while(modIter.hasPrevious())
	{
		ModInfo modInfo = modIter.previous();
		QListIterator<SubmodInfo> submodIter(modInfo.submodList);
		submodIter.toBack();
		while(submodIter.hasPrevious())
		{
			SubmodInfo submodInfo = submodIter.previous();
			for(FileInfo fileInfo: submodInfo.fileList)
			{
				if(!tempList.contains(fileInfo.name))
					filenames.append(fileInfo.name);
				if(submodInfo.isChecked && !tempList.contains(fileInfo.name))
				{
					tempList.append(fileInfo.name);
					filenames.removeAll(fileInfo.name);
					if(!fileMatched(fileInfo))
					{
						installMap.insert(modInfo.name, fileInfo);
						m_dataMap.insert(dataDir->filePath(fileInfo.name), fileInfo.checkSum);
					}
				}
			}
		}
	}

	emit filesCounted(installMap.values().count());
	int count = 0;
	for(QString modName: installMap.uniqueKeys())
	{
		QDir tempDir(QDir::currentPath()+"/temp");
		Archive arc(modsDir->filePath(modName));
		QEventLoop loop;
		connect(&arc, &Archive::filesExtracted, this, [&](QString name, int done) { emit filesCopied(name,count+done); });
		connect(&arc, SIGNAL(extractionFinished()), &loop, SLOT(quit()));
		arc.extract(tempDir.path(), installMap.values(modName));
		loop.exec();
		QFile listFile("listfile.txt");
		listFile.remove();
		count+=installMap.values(modName).count();
		for(FileInfo fileInfo: installMap.values(modName))
		{
			QString oldName = tempDir.filePath(fileInfo.path);
			QString newName = dataDir->filePath(fileInfo.name);
			dataDir->mkpath(newName.section("/", 0, -2));
			QFile::rename(oldName, newName);
		}
		tempDir.removeRecursively();
	}
	for(QString fileName: filenames)
	{
		if(QFile::exists(dataDir->filePath(fileName)))
		{
			QFile::remove(dataDir->filePath(fileName));
			dataDir->rmpath(dataDir->filePath(fileName).section("/", 0, -2));
			m_dataMap.remove(dataDir->filePath(fileName));
		}
	}
	qDebug() << "applying time" << t.elapsed();
}

void ModManager::replyFinished(QHash<QString, NexusModInfo> nexusModInfoIDHash)
{
	qDebug() << "received network reply";
	QHashIterator<QString, NexusModInfo> i(nexusModInfoIDHash);
	while (i.hasNext())
	{
		i.next();
		for(ModInfo &modInfo: m_modList)
			if(modInfo.nexusID == i.key())
				modInfo.latestVersion = i.value().version;
	}
	refreshOnCheck(QModelIndex(), QModelIndex());
/*	QByteArray data = reply->readAll();
	QJsonDocument json = QJsonDocument::fromJson(data);
	int maxwc = 0;
	for(int i = 0; i < json.array().count(); i++)
	{
		QJsonObject object = json.array().at(i).toObject();
		QString name = object.value("name").toString();
		QString uri = reply->request().attribute(QNetworkRequest::User).toString();
		QStringList infolist = name.split(QRegularExpression("[\\W\\d_]+"), QString::SkipEmptyParts);
		QStringList arclist = uri.split(QRegularExpression("[\\W\\d_]+"), QString::SkipEmptyParts);
		//qDebug() << infolist << arclist;
		int wc = 0;
		for(QString word: infolist)
			if(arclist.contains(word))
				wc++;
		if(wc > maxwc)
		{
			maxwc = wc;
			QMutableListIterator<ModInfo> i(m_modList);
			i.findNext({reply->request().attribute(QNetworkRequest::User).toString()});
			ModInfo &modInfo = i.value();
			modInfo.latestVersion = object.value("version").toString();
		}
	}*/
}

void ModManager::exportToModel()
{
	QTime t(0,0);
	t.start();
	m_model->removeRows(0, m_model->rowCount());
	QStringList installList;
	QMultiHash<QString, FileInfo> installMap;
	for(int modGroupRow = m_modGroupList.count()-1; modGroupRow >= 0; modGroupRow--)
	{
		ModGroupInfo modGroupInfo = m_modGroupList.at(modGroupRow);
		for(int modRow = modGroupInfo.modList.count()-1; modRow >= 0; modRow--)
		{
			ModInfo modInfo = modGroupInfo.modList.at(modRow);
			for(int submodRow = modInfo.submodList.count()-1; submodRow >= 0; submodRow--)
			{
				SubmodInfo submodInfo = modInfo.submodList.at(submodRow);
				if(submodInfo.isChecked)
				{
					for(FileInfo fileInfo: submodInfo.fileList)
					{
						if(!installList.contains(fileInfo.name))
						{
							installList.append(fileInfo.name);
							installMap.insert(modInfo.name, fileInfo);
						}
					}
				}
			}
		}
	}
	//qDebug() << "installList";
	QStringList tagList;
	tagList << "Matched" << "Matched Conflict" << "Conflict" << "Mismatched" << "Underwritten" << "Dirty" << "Missing";
	QList<QBrush> colorList;
	if(m_suModeIsEnabled)
		colorList << QBrush(QColor("#BAD696"))<< QBrush(QColor("#BAD696")) << QBrush(QColor("#EDE3A4")) << QBrush(Qt::darkYellow) << QBrush(Qt::darkGreen) << QBrush(Qt::darkGreen) << QBrush(Qt::red);
	else
		colorList << QBrush(QColor("#BAD696"))<< QBrush(QColor("#BAD696")) << QBrush(QColor("#EDE3A4"))  << QBrush(Qt::red) << QBrush(Qt::red) << QBrush(Qt::red) << QBrush(Qt::red);

	for(int modGroupRow = 0; modGroupRow < m_modGroupList.count(); modGroupRow++)
	{
		ModGroupInfo modGroupInfo = m_modGroupList.at(modGroupRow);
		QList<QStandardItem *> modGroupItemList = createModGroupItems(modGroupInfo);
		QStandardItem *modGroupItem = modGroupItemList.first();
		modGroupItem->setData("mgi");
		bool groupHasCheckedSubmods = false;
		bool groupHasUncheckedSubmods = false;
		for(int modRow = 0; modRow < modGroupInfo.modList.count(); modRow++)
		{
			int ci = -1;
			ModInfo modInfo = modGroupInfo.modList.at(modRow);
			QList<QStandardItem *> modItemList = createModItems(modInfo);
			QStandardItem *modItem = modItemList.first();
			modItem->setData("mi");
			if(!modInfo.skipList.isEmpty())
			{
				QStandardItem *skippedItem = new QStandardItem("Skipped");
				skippedItem->setData("skipped", Qt::UserRole+1);
				skippedItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				modItem->appendRow(skippedItem);
				foreach(FileInfo fileInfo, modInfo.skipList)
				{
					QList<QStandardItem *> fileItems = createFileItems(fileInfo);
					skippedItem->appendRow(fileItems);
				}
			}

			bool ModHasCheckedSubmods = false;
			bool ModHasUncheckedSubmods = false;
			for(int submodRow = modInfo.submodList.count()-1; submodRow >= 0; submodRow--)
			{
				SubmodInfo submodInfo = modInfo.submodList.at(submodRow);
				QList<QStandardItem *> submodItems = createSubmodItems(submodInfo);
				QStandardItem* submodItem = submodItems.first();
				submodItem->setData("submod");
				modItem->insertRow(0, submodItems);
				if(submodInfo.isChecked)
				{
					ModHasCheckedSubmods = true;
					groupHasCheckedSubmods = true;
					submodItem->setCheckState(Qt::Checked);
				}
				else
				{
					groupHasUncheckedSubmods = true;
					ModHasUncheckedSubmods = true;
					submodItem->setCheckState(Qt::Unchecked);
				}
				if(modInfo.submodList.count() == 1)
					submodItem->setData(true, Qt::UserRole+2);
				foreach(FileInfo fileInfo, submodInfo.fileList)
				{
					QString group;
					QString secondGroup;
					bool checkSumOk = false;
					for(FileInfo curFileInfo: installMap.values(modInfo.name))
						if(curFileInfo.name == fileInfo.name && curFileInfo.checkSum == fileInfo.checkSum)
						{
							checkSumOk = true;
							break;
						}
					if(checkSumOk)
					{
						if(QFile::exists(dataDir->filePath(fileInfo.name)))
							if(fileMatched(fileInfo))
								group = "Matched";
							else
								group = "Mismatched";
						else
							group = "Missing";
					}
					else
					{
						int markedModGroupRow = -1;
						int markedModRow = -1;
						bool markedMatched = false;
						QHashIterator<QString, FileInfo> iter(installMap);
						while(iter.hasNext())
						{
							iter.next();
							if(iter.value().name == fileInfo.name)
							{
								QPair<int, int> pair = groupAndModIndexFromArchiveName(iter.key());
								markedModGroupRow = pair.first;
								markedModRow = pair.second;
								markedMatched = fileMatched(iter.value());
							}
						}
						if(modGroupRow == markedModGroupRow && modRow == markedModRow)
						{
							if(QFile::exists(dataDir->filePath(fileInfo.name)))
								if(fileMatched(fileInfo))
									if(markedMatched)
										group = "Matched";
									else
										group = "Underwritten";
								else
									if(markedMatched)
										if(m_modGroupList.at(modGroupRow).modList.at(modRow).submodList.at(submodRow).isChecked)
											group = "Internal Conflict";
										else
											group = "Inactive Internal Conflict";
									else
										group = "Mismatched";
							else
								group = "Missing";
						}
						if(modGroupRow > markedModGroupRow || (modGroupRow == markedModGroupRow && modRow > markedModRow))
						{
							if(QFile::exists(dataDir->filePath(fileInfo.name)))
								if(fileMatched(fileInfo))
									if(markedModGroupRow == -1)
										group = "Dirty";
									else
									{
										group = "Matched Conflict";
										secondGroup = m_modGroupList.at(markedModGroupRow).modList.at(markedModRow).name;
									}
								else
									if(markedModGroupRow == -1)
										group = "Mismatched";
									else
									{
										group = "Inactive Conflict";
										secondGroup = m_modGroupList.at(markedModGroupRow).modList.at(markedModRow).name;
									}
							else
								group = "Not Installed";
						}
						if(modGroupRow < markedModGroupRow || (modGroupRow == markedModGroupRow && modRow < markedModRow))
						{
							if(QFile::exists(dataDir->filePath(fileInfo.name)))
								if(fileMatched(fileInfo))
									if(markedMatched)
									{
										group = "Matched Conflict";
										secondGroup = m_modGroupList.at(markedModGroupRow).modList.at(markedModRow).name;
									}
									else
										group = "Underwritten";
								else
									if(markedMatched)
										if(!m_modGroupList.at(modGroupRow).modList.at(modRow).submodList.at(submodRow).isChecked)
										{
											group = "Inactive Conflict";
											secondGroup = m_modGroupList.at(markedModGroupRow).modList.at(markedModRow).name;
										}
										else
										{
											group = "Conflict";
											secondGroup = m_modGroupList.at(markedModGroupRow).modList.at(markedModRow).name;
										}
									else
										group = "Mismatched";
							else
								group = "Not Installed";
						}
					}
					if(tagList.contains(group))
						ci = qMax(ci, tagList.indexOf(group));
					QList<QStandardItem *> fileItems = createFileItems(fileInfo);
					fileItems.at(3)->setText(submodInfo.name);
					QStandardItem *groupItem = nullptr;
					for(int groupRow = 0; groupRow < modItem->rowCount(); groupRow++)
					{
						if(modItem->child(groupRow)->text() == group)
							groupItem = modItem->child(groupRow);
					}
					if(groupItem == nullptr)
					{
						groupItem = new QStandardItem(group);
						groupItem->setData("fileOrGroup", Qt::UserRole+1);
						groupItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
						modItem->appendRow(groupItem);
					}
					if(secondGroup.isEmpty())
						groupItem->appendRow(fileItems);
					else
					{
						QStandardItem *secGroupItem = nullptr;
						for(int groupRow = 0; groupRow < groupItem->rowCount(); groupRow++)
						{
							if(groupItem->child(groupRow)->text() == secondGroup)
								secGroupItem = groupItem->child(groupRow);
						}
						if(secGroupItem == nullptr)
						{
							secGroupItem = new QStandardItem(secondGroup);
							secGroupItem->setData("fileOrGroup", Qt::UserRole+1);
							secGroupItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
							groupItem->appendRow(secGroupItem);
						}
						secGroupItem->appendRow(fileItems);
					}
				}
			}
			if(ModHasCheckedSubmods && !ModHasUncheckedSubmods)
				modItem->setCheckState(Qt::Checked);
			if(!ModHasCheckedSubmods && ModHasUncheckedSubmods)
				modItem->setCheckState(Qt::Unchecked);
			if(ModHasCheckedSubmods && ModHasUncheckedSubmods)
				modItem->setCheckState(Qt::PartiallyChecked);
			if(ci != -1)
			{
				for(QStandardItem *item: modItemList)
					item->setBackground(colorList.at(ci));
			}
			if(modGroupInfo.modList.count() > 1)
				modGroupItem->appendRow(modItemList);
			else
				m_model->appendRow(modItemList);
		}
		if(modGroupInfo.modList.count() > 1)
		{
			if(groupHasCheckedSubmods && !groupHasUncheckedSubmods)
				modGroupItem->setCheckState(Qt::Checked);
			if(!groupHasCheckedSubmods && groupHasUncheckedSubmods)
				modGroupItem->setCheckState(Qt::Unchecked);
			if(groupHasCheckedSubmods && groupHasUncheckedSubmods)
				modGroupItem->setCheckState(Qt::PartiallyChecked);
			m_model->appendRow(modGroupItemList);
		}
	}
	qDebug() << "export time" << t.elapsed();
	emit finishedExporting();
}

void ModManager::syncSelection(QModelIndex index)
{
	qDebug() << "syncing Selection";
	if(!index.isValid())
		return;
	QStandardItem* item = m_model->itemFromIndex(modFilter->mapToSource(index));
	bool isChecked = item->checkState() == Qt::PartiallyChecked || item->checkState() == Qt::Checked;
	qDebug() << "sync item is" << item->text();
	if(index.data(Qt::UserRole+1).toString() == "mgi")
	{
		ModGroupInfo &modGroupInfo = m_modGroupList[index.row()];
		for(ModInfo &modInfo: modGroupInfo.modList)
		{
			for(SubmodInfo &submodInfo: modInfo.submodList)
			{
				submodInfo.isChecked = isChecked;
			}
		}
	}
	if(index.data(Qt::UserRole+1).toString() == "mi")
	{
		if(index.parent().isValid())
		{
			ModGroupInfo &modGroupInfo = m_modGroupList[index.parent().row()];
			for(SubmodInfo &submodInfo: modGroupInfo.modList[index.row()].submodList)
			{
				submodInfo.isChecked = isChecked;
			}
		}
		else
		{
			for(SubmodInfo &submodInfo: m_modGroupList[index.row()].modList[0].submodList)
			{
				submodInfo.isChecked = isChecked;
			}
		}
	}
	if(index.data(Qt::UserRole+1).toString() != "mi" && index.data(Qt::UserRole+1).toString() != "mgi")
	{
		QModelIndex mi = index.parent();
		if(mi.parent().isValid())
		{
			ModGroupInfo &modGroupInfo = m_modGroupList[mi.parent().row()];
			modGroupInfo.modList[mi.row()].submodList[index.row()].isChecked = isChecked;
		}
		else
		{
			m_modGroupList[index.row()].modList[0].submodList[index.row()].isChecked = isChecked;
		}
	}
}

void ModManager::refreshOnCheck(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	qDebug() << "refreshOnCheck" << modFilter->rowCount() << modsDir->count();
	if(modFilter->rowCount() > modsDir->count())
		return;

	disconnect(modFilter, &SortFilterProxyModel::dataChanged, this, &ModManager::refreshOnCheck);
	disconnect(modFilter, &SortFilterProxyModel::rowsInserted, this, &ModManager::refreshOnDrop);

	//syncSelection(topLeft);
	//applyChanges();
	//exportToModel();

	connect(modFilter, &SortFilterProxyModel::dataChanged, this, &ModManager::refreshOnCheck);
	connect(modFilter, &SortFilterProxyModel::rowsInserted, this, &ModManager::refreshOnDrop, Qt::QueuedConnection);
}

QPair<int, int> ModManager::groupAndModIndexFromArchiveName(QString archiveName)
{
	for(int modGroupRow = 0; modGroupRow < m_modGroupList.count(); modGroupRow++)
	{
		for(int modRow = 0; modRow < m_modGroupList.at(modGroupRow).modList.count(); modRow++)
		{
			if(m_modGroupList.at(modGroupRow).modList.at(modRow).name == archiveName)
				return qMakePair(modGroupRow, modRow);
		}

	}
	//qDebug() << "ERRORan" << archiveName;
	return qMakePair(-1, -1);
}

int ModManager::indexFromModGroupName(QString groupName)
{
	for(int modGroupRow = 0; modGroupRow < m_modGroupList.count(); modGroupRow++)
		if(m_modGroupList.at(modGroupRow).name == groupName)
			return modGroupRow;
	//qDebug() << "ERRORan" << archiveName;
	return -1;
}

int ModManager::indexFromFileName(QString fileName, ModInfo modInfo)
{
	for(int i = 0; i < modInfo.submodList.count(); i++)
		if(modInfo.submodList.at(i).name == fileName)
			return i;
	//qDebug() << "ERRORfn" << fileName;
	return -1;
}
