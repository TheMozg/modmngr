#include "bashimportwizardpage.h"
#include "ui_bashimportwizardpage.h"
#include <QtWidgets>

BashImportWizardPage::BashImportWizardPage(QWidget *parent) :
	QWizardPage(parent),
	ui(new Ui::BashImportWizardPage)
{
	ui->setupUi(this);
	connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &BashImportWizardPage::completeChanged);
}

BashImportWizardPage::~BashImportWizardPage()
{
	delete ui;
}

bool BashImportWizardPage::validatePage()
{
	QString textstr = ui->plainTextEdit->toPlainText();
	QStringList list = textstr.split("\n");
	qDebug() << list;
	QRegularExpression regex2("(.*\\.zip|.*\\.7z)", QRegularExpression::CaseInsensitiveOption);
	for(QString line: list)
	{
		if(line.startsWith("++ "))
		{
			line = regex2.match(line, 9).captured(1);
			installed.append(qMakePair(line, true));
		}
		if(line.startsWith("-- "))
		{
			line = regex2.match(line, 9).captured(1);
			installed.append(qMakePair(line, false));
		}
	}
	return true;
}

bool BashImportWizardPage::isComplete() const
{
	return !ui->plainTextEdit->toPlainText().isEmpty();
}
