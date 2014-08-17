#ifndef BASHIMPORTWIZARDPAGE_H
#define BASHIMPORTWIZARDPAGE_H

#include <QWizardPage>

namespace Ui {
	class BashImportWizardPage;
}

class BashImportWizardPage : public QWizardPage
{
	Q_OBJECT

public:
	explicit BashImportWizardPage(QWidget *parent = 0);
	~BashImportWizardPage();
	bool validatePage() Q_DECL_OVERRIDE;
	QList<QPair<QString, bool>> installed;
	bool isComplete() const Q_DECL_OVERRIDE;

private:
	QStringList res;
	Ui::BashImportWizardPage *ui;
};

#endif // BASHIMPORTWIZARDPAGE_H
