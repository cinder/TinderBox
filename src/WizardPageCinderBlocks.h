#ifndef WizardPageCinderBlocks_H
#define WizardPageCinderBlocks_H

#include <QWizardPage>
#include <QStyledItemDelegate>
#include "CinderBlock.h"
#include "MainWizard.h"

namespace Ui {
class WizardPageCinderBlocks;
}

class CinderBlockItemDelegate : public QStyledItemDelegate
 {
	 Q_OBJECT
 public:
	 CinderBlockItemDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

	 virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
				const QModelIndex &index) const;
	 virtual QSize sizeHint(const QStyleOptionViewItem &/*option*/,
					const QModelIndex &/*index*/) const { return QSize( 200, 54 ); }
};

class WizardPageCinderBlocks : public QWizardPage
{
	Q_OBJECT
	
public:
	explicit WizardPageCinderBlocks( MainWizard *parent = 0);
	~WizardPageCinderBlocks();
	
	void		setCinderLocation( const QString &path );
	
public slots:
	void	selectionChanged();
	void	installTypeChanged();
	void	itemDoubleClicked(class QListWidgetItem*);

private slots:
	void on_showErrorsButton_clicked();
	
private:
	MainWizard			*mParent;
	CinderBlock*		getCurrentCinderBlock();

	Ui::WizardPageCinderBlocks		*ui;
	QString					mOldCinderPath;

	void	updateCinderBlockList();
};

#endif // WizardPageCinderBlocks_H
