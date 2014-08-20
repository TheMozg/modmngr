#include "archiver.h"

QString Archive::relativePath(QString filePath)
{
	if(filePath.endsWith(".esp") || filePath.endsWith(".esm") || filePath.endsWith(".bsa"))
	{
		return filePath.section("/", -1);
	}
	QStringList tagList;
	tagList = {"meshes", "interface", "strings", "grass", "scripts", "shadersfx", "music", "sound", "textures", "video", "seq"};
	QStringList spltstr = filePath.split("/");
	int firstIndex = spltstr.count();
	foreach (QString tag, tagList)
	{
		if(spltstr.contains(tag))
			if(spltstr.indexOf(tag) < firstIndex)
				firstIndex = spltstr.indexOf(tag);
	}
	if(firstIndex != spltstr.count())
	{
		for(int i = 0; i < firstIndex; i++)
			spltstr.removeFirst();
		return spltstr.join("/");
	}
	return "";
}

QString Archive::subDir(QString filePath, QString relativeFilePath)
{
	if(relativeFilePath.isEmpty())
		return QString();
	if(relativeFilePath == filePath)
		return "(root)";
	return filePath.remove(relativeFilePath).section("/",-1,-1,QString::SectionSkipEmpty);
}

QList<ModInfo> Archive::list(QStringList mylist)
{
	QTime t(0,0);
	t.start();
	QFile listFile("listfile.txt");
	listFile.open(QFile::WriteOnly | QFile::Text);
	QTextStream fs(&listFile);
	fs.setCodec("UTF-8");
	for(QString fileInfo: mylist)
			fs << fileInfo << endl;
	listFile.close();
	QString exePath = QDir::current().filePath("7za.exe");
	QProcess* process = new QProcess();
	process->setProgram(exePath);
	process->setStandardOutputFile("list.txt");
	process->setWorkingDirectory(QDir::current().path());
	process->setNativeArguments("l -slt -an -ai@listfile.txt");
	process->start();
	process->waitForFinished(-1);
	//qDebug() << exePath << process->error();
	QFile *file = new QFile("list.txt");
	file->open(QFile::ReadOnly | QFile::Text);
	QStringList output;
	while (!file->atEnd())
		output.append(file->readLine());
	file->remove();
	listFile.remove();
	//qDebug() << output;
	QList<ModInfo> modInfoList;
	int i = 3;
	while(i < output.count())
	{
		ModInfo modInfo;
		modInfo.name = output.at(i).trimmed().mid(17).section("\\", -1);
		int a = 10;
		int b = 4;
		int c = 5;
		int d = 12;
		if(modInfo.name.endsWith("zip"))
		{
			a = 15;
			b = 7;
			c = 10;
			d = 8;
		}
		i+=d;
		QMap<QString, SubmodInfo> sortMap;
		while(i < output.count() && output.at(i).trimmed().startsWith("Path"))
		{
			if(output.at(i+b).trimmed().at(13) == 'D')
			{
				i+=a;
				continue;
			}
			FileInfo fileInfo;
			fileInfo.path = QDir::fromNativeSeparators(output.at(i).trimmed().mid(7)).toLower();
			fileInfo.name = relativePath(fileInfo.path);
			fileInfo.checkSum = output.at(i+c).trimmed().mid(6);
			if(fileInfo.checkSum.isEmpty())
				fileInfo.checkSum = "(none)";
			QString submod = subDir(fileInfo.path, fileInfo.name);
			if(submod.isEmpty())
			{
				fileInfo.name = fileInfo.path.section("/", -1);
				modInfo.skipList.append(fileInfo);
			}
			else
			{
				if(!sortMap.contains(submod))
					sortMap.insert(submod, {submod, false, QList<FileInfo>()});
				sortMap[submod].fileList.append(fileInfo);
			}
			i+=a;
		}
		modInfo.submodList = sortMap.values();
		modInfoList.append(modInfo);
		i+=1;
	}
	qDebug() << "listing time" << t.elapsed();
	return modInfoList;
}

Archive::Archive(QString pathToZip)
{
	exePath = QDir::current().filePath("7za.exe");
	path = pathToZip;
	QFileInfo exeInfo(exePath);
	process = new QProcess();
	process->setWorkingDirectory(exeInfo.path());
	timer = new QTimer();
	signalMapper = new QSignalMapper();
	connect(timer, SIGNAL(timeout()), signalMapper, SLOT(map()));
	connect(signalMapper, SIGNAL(mapped(QString)),this, SLOT(update(QString)));
}

void Archive::extract(QString outputPath, QList<FileInfo> fileList)
{
	total = fileList.count();
	QFile listFile("listfile.txt");
	listFile.open(QFile::WriteOnly | QFile::Text);
	QTextStream fs(&listFile);
	fs.setCodec("UTF-8");
	for(FileInfo fileInfo: fileList)
		fs << QDir::toNativeSeparators(fileInfo.path) << endl;
	listFile.close();

	signalMapper->setMapping(timer, outputPath);
	timer->start(100);
	QStringList arguments;
	arguments <<"x" << path << "-y" << "-o"+outputPath << "@"+listFile.fileName();
	process->start(exePath, arguments);
	//listFile.remove();
}

void Archive::update(QString outputPath)
{
	if(process->state() == QProcess::NotRunning)
	{
		timer->stop();
		emit filesExtracted(path.section("/",-1), total);
		emit extractionFinished();
		return;
	}
	quint64 count = 0;
	QDirIterator it(outputPath, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		it.next();
		count++;
	};
	qDebug() <<"extracted" << count;
	if(count != prcount)
	{
		prcount = count;
		emit filesExtracted(path.section("/",-1), count);
	}
}

QString Archive::getFileCRC(QString fileName)
{
	QProcess process;
	QStringList arguments;
	arguments << QDir::toNativeSeparators(fileName);
	process.start(QDir::current().filePath("crc32.exe"), arguments);
	process.waitForFinished(-1);
	QString line = process.readLine().trimmed();
	return line.section(' ', -1);
}

QList<ModInfo> Archive::extractIdAndVersion(QString outputPath, QList<ModInfo> modInfoList, QDir dir)
{
	QList<ModInfo> reslist;
	QFile listFile("listfile.txt");
	listFile.open(QFile::WriteOnly | QFile::Text);
	QTextStream fs(&listFile);
	for(ModInfo modInfo: modInfoList)
			fs << dir.filePath(modInfo.name) << endl;
	listFile.close();
	QStringList args = {"e", "-an", "-y", "-o"+outputPath+"\\*", "info.xml", "-r", "-ai@listfile.txt"};
	QString exePath = QDir::current().filePath("7za.exe");
	QFileInfo exeInfo(exePath);
	QProcess* process = new QProcess();
	process->setWorkingDirectory(exeInfo.path());
	process->start(exePath, args);
	process->waitForFinished(-1);
	qDebug() << process->readAll();
	for(ModInfo modInfo: modInfoList)
	{
		QFileInfo mfi(modInfo.name);
		if(QFile::exists(outputPath+"/"+ mfi.baseName()))
		{
			ModInfo resm;
			QFile file(outputPath+"/"+mfi.baseName()+"/info.xml");
			file.open(QFile::ReadOnly);
			QXmlStreamReader xml(&file);
			while (!xml.atEnd())
			{
				xml.readNext();
				if(xml.isStartElement())
				{
					if(xml.name().toString() == "Name")
						resm.name = xml.readElementText();
					if(xml.name().toString() == "Version")
						resm.version = xml.readElementText();
					if(xml.name().toString() == "Website")
						resm.nexusID = xml.readElementText();
				}
			}
			file.close();
			qDebug() << resm.version;
			reslist.append(resm);
		}
	}
	return reslist;
}
