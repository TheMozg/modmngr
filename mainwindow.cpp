#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sortfilterproxymodel.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::updateFileView()
{
	if(ui->treeView->model() == nullptr)
	{
		ui->treeView->setModel(modManager->fileFilter);
	}
	QModelIndexList list = ui->modsView->selectionModel()->selectedRows();

	if(list.count() == 1)
	{
		if(list.first().data(Qt::UserRole+1) == "mgi")
			ui->treeView->setModel(nullptr);
		else
			ui->treeView->setRootIndex(modManager->fileFilter->mapFromSource(modManager->modFilter->mapToSource(list.first())));
	}
	else
	{
		ui->treeView->setModel(nullptr);
	}
}

QString MainWindow::getValue(QString key)
{
	QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE", QSettings::NativeFormat);
	QString key64 = "Wow6432Node/" + key;
	if(settings.value(key).isValid())
		return QDir::fromNativeSeparators(settings.value(key).toString());
	if(settings.value(key64).isValid())
		return QDir::fromNativeSeparators(settings.value(key64).toString());
	return "";
}

void MainWindow::on_bossButton_clicked()
{
	dialog = new QProgressDialog("Sorting mods with BOSS...", "Cancel", 0, 1, this,
								 Qt::Dialog |Qt::WindowTitleHint| Qt::MSWindowsFixedSizeDialogHint|Qt::WindowCloseButtonHint);
	dialog->setWindowModality(Qt::WindowModal);
	QProgressBar *pb = new QProgressBar(dialog);
	pb->setTextVisible(false);
	dialog->setBar(pb);
	dialog->setWindowTitle(windowTitle());
	dialog->setMinimumDuration(0);
	dialog->setRange(0,0);
	dialog->show();
	QString bossExePath = m_bossDirPath + "/BOSS.exe";
	if(QFile::exists(bossExePath))
	{
		process = new QProcess();
		process->setWorkingDirectory(m_bossDirPath);
		connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(bossfin()));
		connect(dialog, &QProgressDialog::canceled, this, &MainWindow::bosscancel);
		process->start(bossExePath, QStringList());
	}
}

void MainWindow::on_actionOpenGameFolder_triggered()
{
	QString path = QDir::toNativeSeparators(skyrimPath);
	QProcess::startDetached("explorer.exe",  QStringList() << path);
}

void MainWindow::on_actionLaunch_triggered()
{
	QString path = skyrimPath + "/SkyrimLauncher.exe";
	QProcess::startDetached(path);
}

void MainWindow::on_actionLaunchSkse_triggered()
{
	QString path = skyrimPath + "/skse.exe";
	QProcess::startDetached(path);
}

void MainWindow::on_actionSuMode_triggered()
{
	updateUi();
}

void MainWindow::on_actionChangeSkyrim_triggered()
{
	QWizard *wiz = new QWizard(this,Qt::WindowCloseButtonHint|Qt::CustomizeWindowHint);
	wiz->setWindowTitle("SMM Setup");
	WizardPage *dataPage = new WizardPage(QStringList() << "TESV.exe" << "data", skyrimPath, "data");
	dataPage->setTitle("Select Skyrim directory");
	dataPage->setSubTitle("Please select the directory of your Skyrim installation. It is the one containing TESV.exe and data folder.");
	wiz->addPage(dataPage);
	wiz->exec();
	if(wiz->result() == 1)
	{
		skyrimPath = wiz->field("data").toString();
		modManager->setPaths(skyrimPath, modsPath);
		pluginManager->setSkyrimPath(skyrimPath);
		pluginManager->reload();
		pluginManager->save();
		updateUi();
	}
	for(int id: wiz->pageIds())
		wiz->removePage(id);
}

void MainWindow::on_actionChangeMods_triggered()
{
	QWizard *wiz = new QWizard(this,Qt::WindowCloseButtonHint|Qt::CustomizeWindowHint);
	wiz->setWindowTitle("SMM Setup");
	WizardPage *modPage = new WizardPage(QStringList(), modsPath, "mod");
	modPage->setTitle("Select mods directory");
	modPage->setSubTitle("Please select the direcory where all your mods are located.");
	wiz->addPage(modPage);
	wiz->exec();
	if(wiz->result() == 1)
	{
		modsPath = wiz->field("mod").toString();
		modManager->setPaths(skyrimPath, modsPath);
		pluginManager->setSkyrimPath(skyrimPath);
		pluginManager->reload();
		pluginManager->save();
		updateUi();
	}
	for(int id: wiz->pageIds())
		wiz->removePage(id);
}

void MainWindow::on_actionChangeBoss_triggered()
{
	QWizard *wiz = new QWizard(this,Qt::WindowCloseButtonHint|Qt::CustomizeWindowHint);
	wiz->setWindowTitle("SMM Setup");
	WizardPage *modPage = new WizardPage({"boss.exe"}, m_bossDirPath, "boss");
	modPage->setTitle("Select boss directory");
	modPage->setSubTitle("Please select the direcory where BOSS is located.");
	wiz->addPage(modPage);
	wiz->exec();
	if(wiz->result() == 1)
	{
		m_bossDirPath = wiz->field("boss").toString();
		updateUi();
	}
	for(int id: wiz->pageIds())
		wiz->removePage(id);
}

void MainWindow::on_actionAbout_triggered()
{
	QMessageBox::about(this, "About Skyrim Mod Manager", "Designed by TheMozg, v1.0");
}

void MainWindow::saveSettings()
{
	QFile file(QDir::current().filePath("settings.xml"));
	file.open(QFile::WriteOnly | QFile::Text);
	QXmlStreamWriter stream(&file);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();
	stream.writeStartElement("Settings");

	stream.writeStartElement("suMode");
	stream.writeAttribute("isEnabled", QString::number(modManager->m_suModeIsEnabled, 2));
	stream.writeEndElement();

	stream.writeStartElement("Skyrim");
	stream.writeAttribute("Path", skyrimPath);
	stream.writeEndElement();

	stream.writeStartElement("Mods");
	stream.writeAttribute("Path", modsPath);
	stream.writeEndElement();

	stream.writeStartElement("Boss");
	stream.writeAttribute("Path", m_bossDirPath);
	stream.writeEndElement();

	stream.writeStartElement("updateOnLaunch");
	stream.writeAttribute("isEnabled", QString::number(ui->actionOn_app_launch->isChecked(), 2));
	stream.writeEndElement();

	stream.writeStartElement("LastUpdate");
	stream.writeAttribute("Date", lastUpdate.toString());
	stream.writeEndElement();

	stream.writeEndElement();
	stream.writeEndDocument();
	file.close();
}

void MainWindow::updateUi()
{
	bool skseExists = QFile::exists(skyrimPath + "/skse.exe");
	bool skyrimExists = QFile::exists(skyrimPath + "/TESV.exe");
	bool bossExists = QFile::exists(m_bossDirPath + "/boss.exe");
	ui->actionLaunchSkse->setEnabled(skseExists);
	ui->actionLaunch->setEnabled(skyrimExists);
	ui->actionOpenGameFolder->setEnabled(skyrimExists);
	ui->bossButton->setEnabled(bossExists);
	modManager->m_suModeIsEnabled = ui->actionSuMode->isChecked();
	ui->treeView->setHidden(!ui->actionSuMode->isChecked());
}

void MainWindow::loadSettings()
{
	QFile file(QDir::current().filePath("settings.xml"));
	file.open(QFile::ReadOnly);
	QXmlStreamReader xml(&file);
	while (!xml.atEnd())
	{
		xml.readNext();
		if(xml.isStartElement())
		{
			if(xml.name().toString() == "Skyrim")
				skyrimPath = xml.attributes().value("Path").toString();
			if(xml.name().toString() == "Mods")
				modsPath = xml.attributes().value("Path").toString();
			if(xml.name().toString() == "Boss")
				m_bossDirPath = xml.attributes().value("Path").toString();
			if(xml.name().toString() == "suMode")
				ui->actionSuMode->setChecked(xml.attributes().value("isEnabled").toInt());
			if(xml.name().toString() == "LastUpdate")
				lastUpdate = QDate::fromString(xml.attributes().value("Date").toString());
			if(xml.name().toString() == "updateOnLaunch")
			{
				if(xml.attributes().value("isEnabled").toInt())
					ui->actionOn_app_launch->trigger();
				else
					ui->actionOnce_a_day->trigger();
			}
		}
	}
	file.close();
}

bool MainWindow::init()
{
	ui->actionOnce_a_day->setChecked(true);

	connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::closing);

	loadSettings();

	QWizard *wiz = new QWizard(this,Qt::WindowCloseButtonHint|Qt::CustomizeWindowHint);
	wiz->setWindowTitle("SMM Setup");
	WizardPage *modPage = new WizardPage(QStringList(), modsPath, "mod");
	modPage->setTitle("Select mods directory");
	modPage->setSubTitle("Please select the direcory where all your mods are located.");
	WizardPage *dataPage = new WizardPage(QStringList() << "TESV.exe" << "data", skyrimPath, "data");
	dataPage->setTitle("Select Skyrim directory");
	dataPage->setSubTitle("Please select the directory of your Skyrim installation. It is the one containing TESV.exe and data folder.");
	if(!(QFile::exists(skyrimPath + "/TESV.exe") && QFile::exists(skyrimPath + "/data")))
	{
		skyrimPath = getValue("bethesda softworks/skyrim/installed path");
		if(!(QFile::exists(skyrimPath + "/TESV.exe") && QFile::exists(skyrimPath + "/data")))
		{
			wiz->addPage(dataPage);
		}
	}
	if(!QFile::exists(modsPath))
		wiz->addPage(modPage);
	if(!wiz->pageIds().isEmpty())
	{
		wiz->exec();
		if(wiz->result() == 1)
		{
			if(wiz->field("data").isValid())
				skyrimPath = wiz->field("data").toString();
			if(wiz->field("mod").isValid())
				modsPath = wiz->field("mod").toString();
		}
		else
		{
			return false;
		}
	}
	for(int id: wiz->pageIds())
		wiz->removePage(id);

	if(!QFile::exists(m_bossDirPath + "/boss.exe"))
		m_bossDirPath = getValue("BOSS/installed path");


	pluginManager = new PluginManager(this);
	pluginManager->setSkyrimPath(skyrimPath);
	ui->pluginsView->setModel(pluginManager->m_model);
	ui->pluginsView->addAction(ui->actionCheckSelection);
	ui->pluginsView->addAction(ui->actionUncheckSelectio);

	modManager = new ModManager(this);
	connect(modManager, &ModManager::finishedExporting, this, &MainWindow::finishInstalling);

	updateUi();

	ui->modsView->setModel(modManager->modFilter);
	ui->modsView->header()->setSectionsMovable(false);
	ui->modsView->addAction(ui->actionInstallSelection);
	ui->modsView->addAction(ui->actionUninstallSelection);
	connect(ui->modsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateFileView);
	ui->treeView->setModel(modManager->fileFilter);
	ui->treeView->header()->setSectionsMovable(false);

	loadDialog = createDialog("Loading mods", 0);
	connect(modManager, &ModManager::fileLoaded, this, &MainWindow::updateLoadDialog);

	mydialog2 = createDialog("Instaling mods");
	connect(modManager, &ModManager::filesCopied, this, &MainWindow::updateDialog);
	connect(modManager, &ModManager::filesCounted, this, &MainWindow::setDialogMax);

	modManager->setPaths(skyrimPath, modsPath);

	updateFileView();
	if(ui->actionOn_app_launch->isChecked() || lastUpdate.daysTo(QDate::currentDate()) > 0)
	{
		modManager->accessNexusDatabase();
		lastUpdate = QDate::currentDate();
	}
	return true;
}

void MainWindow::bossfin()
{
	dialog->reset();
	pluginManager->reload();
	pluginManager->save();
}

void MainWindow::bosscancel()
{
	process->disconnect(this);
	process->kill();
	pluginManager->reload();
	pluginManager->save();
}

void MainWindow::finishInstalling()
{
	mydialog2->setValue(mydialog2->maximum());
	pluginManager->reload();
	pluginManager->save();
}

void MainWindow::setDialogMax(int count)
{
	mydialog2->setMaximum(count+1);
	mydialog2->setValue(0);
}

void MainWindow::updateDialog(QString modName,int done)
{
	qDebug() <<"dialog value" << mydialog2->value() << done << mydialog2->maximum();
	if(done==mydialog2->maximum()-1)
		mydialog2->setLabelText("Analizing mods");
	else
		mydialog2->setLabelText(modName);
	mydialog2->setValue(done);
}

void MainWindow::updateLoadDialog(QString modName)
{
	loadDialog->setLabelText(modName);
	loadDialog->setValue(loadDialog->value()+1);
}

void MainWindow::closing()
{
	modManager->saveDataCash(QDir::current().filePath("datacash.txt"));
	modManager->save(QDir::current().filePath("modlist.txt"));
	pluginManager->save();
	saveSettings();
}

QProgressDialog *MainWindow::createDialog(QString title, int max)
{
	QProgressDialog *dialog = new QProgressDialog(title, "Cancel", 0, max, this,
										Qt::Dialog|Qt::WindowTitleHint|Qt::MSWindowsFixedSizeDialogHint);
	dialog->setWindowModality(Qt::WindowModal);
	dialog->setWindowTitle(title);
	dialog->setMinimumDuration(0);
	dialog->setCancelButton(NULL);
	return dialog;
}

void MainWindow::on_actionOpenModsFolder_triggered()
{
	QString path = QDir::toNativeSeparators(modsPath);
	QProcess::startDetached("explorer.exe",  QStringList() << path);
}

void MainWindow::on_actionInstallSelection_triggered()
{
	modManager->checkSelection(ui->modsView->selectionModel()->selectedRows());
}

void MainWindow::on_actionUninstallSelection_triggered()
{
	modManager->unCheckSelection(ui->modsView->selectionModel()->selectedRows());
}

void MainWindow::on_actionCheck_now_triggered()
{
	modManager->accessNexusDatabase();
	lastUpdate = QDate::currentDate();
}

void MainWindow::on_actionOnce_a_day_triggered(bool checked)
{
	if(checked)
		ui->actionOn_app_launch->setChecked(false);
	else
		ui->actionOnce_a_day->setChecked(true);
}

void MainWindow::on_actionOn_app_launch_triggered(bool checked)
{
	if(checked)
		ui->actionOnce_a_day->setChecked(false);
	else
		ui->actionOn_app_launch->setChecked(true);
}

void MainWindow::on_actionReadme_triggered()
{
	QProcess::startDetached("notepad.exe",  QStringList() << QDir::current().filePath("readme.txt"));
}

void MainWindow::on_actionCheckSelection_triggered()
{
	pluginManager->checkSelection(ui->pluginsView->selectionModel()->selectedRows());
}

void MainWindow::on_actionUncheckSelectio_triggered()
{
	pluginManager->unCheckSelection(ui->pluginsView->selectionModel()->selectedRows());
}
