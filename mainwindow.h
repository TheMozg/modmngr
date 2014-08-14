#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pluginmanager.h"
#include "modmanager.h"
#include "sortfilterproxymodel.h"
#include "wizardpage.h"
#include <QtWidgets>

namespace Ui
{
	class MainWindow;
}

class MainWindow: public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	bool init();

private slots:
	void updateFileView();
	void on_bossButton_clicked();
	void on_actionOpenGameFolder_triggered();
	void on_actionLaunch_triggered();
	void on_actionLaunchSkse_triggered();
	void on_actionSuMode_triggered();
	void on_actionChangeSkyrim_triggered();
	void on_actionChangeMods_triggered();
	void on_actionChangeBoss_triggered();
	void on_actionAbout_triggered();
	void bossfin();
	void bosscancel();
	void finishInstalling();
	void setDialogMax(int count);
	void updateDialog(QString modName, int done);
	void updateLoadDialog(QString modName);
	void closing();
	void on_actionOpenModsFolder_triggered();
	void on_actionInstallSelection_triggered();
	void on_actionUninstallSelection_triggered();
	void on_actionCheck_now_triggered();
	void on_actionOnce_a_day_triggered(bool checked);
	void on_actionOn_app_launch_triggered(bool checked);

	void on_actionReadme_triggered();

	void on_actionCheckSelection_triggered();

	void on_actionUncheckSelectio_triggered();

private:
	QDate lastUpdate;
	Ui::MainWindow *ui;
	QString skyrimPath;
	QString modsPath;
	QProgressDialog *createDialog(QString title, int max = 1);
	void loadSettings();
	void saveSettings();
	void updateUi();
	PluginManager* pluginManager;
	ModManager *modManager;
	QString getValue(QString key);
	QString m_bossDirPath;
	QProgressDialog* dialog;
	QProgressDialog *loadDialog;
	QProgressDialog* mydialog2;
	QProcess *process;
};

#endif // MAINWINDOW_H
