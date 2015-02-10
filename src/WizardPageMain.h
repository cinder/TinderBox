#ifndef WizardPageMain_H
#define WizardPageMain_H

#include <QWizardPage>

#include "TinderBox.h"
#include "Preferences.h"
#include "ProjectTemplate.h"

class MainWizard;

namespace Ui {
class WizardPageMain;
}

class WizardPageMain : public QWizardPage
{
	Q_OBJECT
	
public:
	explicit WizardPageMain( MainWizard *parent = 0);
	~WizardPageMain();

	const ProjectTemplate&	getProjectTemplate() const;
	QString					getProjectName() const;
	QString					getLocation() const;
	QString					getCinderLocation() const;
	bool					shouldCreateGitRepo() const;

	void	enablePlatform( size_t platform, bool enable );
	bool	isXcodeSelected() const;
	bool	isXcodeIosSelected() const;
	bool	isVc2012WinrtSelected() const;
	bool	isVc2013Selected() const;

	void                updateCinderVersionsCtrl();
	void				updateTemplates();
	virtual bool		isComplete() const;
	virtual void		initializePage();
protected:
    virtual void        closeEvent( QCloseEvent *event );
	
private:
	Ui::WizardPageMain	*ui;
	MainWizard		*mParent;

	QList<ProjectTemplate>					mProjectTemplates;
    QPalette                                mLocationPaletteOrig;
    QList<QMap<QString,QString> >			mPlatformConditions;

	QString				getTemplateValue() const;
	void				updateProjectNameStatus();
	void                validateNextButton();

private slots:
    void on_compilerList_itemSelectionChanged();
    void on_locationLineEdit_textChanged( QString text );
    void on_projectNameLineEdit_textChanged( QString text );
	void on_cinderVersionComboBox_activated( QString value );
    void on_locationButton_clicked();
	void on_templateComboBox_currentIndexChanged(int index);
	void on_showErrorsButton_clicked();
};

#endif // WizardPageMain_H
