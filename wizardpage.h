#ifndef WIZARDPAGE_H
#define WIZARDPAGE_H

#include <QWizardPage>

namespace Ui {
class WizardPage;
}

class WizardPage : public QWizardPage
{
	Q_OBJECT

public:
	explicit WizardPage(QStringList checklist, QString defaultPath, QString name, QWidget *parent = 0);
	~WizardPage();

	void initializePage() Q_DECL_OVERRIDE;
	bool isComplete() const Q_DECL_OVERRIDE;

private slots:
	void on_toolButton_clicked();

private:
	Ui::WizardPage *ui;
	QStringList m_checkList;
	QString m_defaultPath;
};

#endif // WIZARDPAGE_H
