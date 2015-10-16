/*
 Copyright (c) 2015, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and
    the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
    the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

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
