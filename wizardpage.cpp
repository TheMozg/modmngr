#include "wizardpage.h"
#include "ui_wizardpage.h"
#include <QtWidgets>

WizardPage::WizardPage(QStringList checklist, QString defaultPath, QString name, QWidget *parent): QWizardPage(parent), ui(new Ui::WizardPage)
{
	ui->setupUi(this);
	registerField(name, ui->lineEdit);
	connect(ui->lineEdit, &QLineEdit::textChanged, this, &WizardPage::completeChanged);
	m_checkList = checklist;
	m_defaultPath = QDir::cleanPath(defaultPath);
}

WizardPage::~WizardPage()
{
	delete ui;
}

void WizardPage::on_toolButton_clicked()
{
	 QString path = QFileDialog::getExistingDirectory(this, title(), QDir::homePath());
	 if(!path.isEmpty())
		ui->lineEdit->setText(path);
}

void WizardPage::initializePage()
{
	ui->lineEdit->setText(m_defaultPath);
}

bool WizardPage::isComplete() const
{
	if(ui->lineEdit->text().isEmpty())
		return false;
	QDir dir(ui->lineEdit->text());
	bool ok = dir.exists() && dir.path() != m_defaultPath;
	for(QString relPath: m_checkList)
	{
		if(!dir.exists(relPath))
			ok = false;
	}
	if(ok)
		return true;
	else
		return false;
}
