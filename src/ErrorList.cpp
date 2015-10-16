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

#include "ErrorList.h"
#include "ui_ErrorListDialog.h"

#include <QStandardItemModel>

void ErrorList::addWarning( const QString &str, const QString &filePath )
{
	if( filePath.isEmpty() )
		mMessages.push_back( Error( false, str, mActiveFilePath ) );
	else
		mMessages.push_back( Error( false, str, filePath ) );
}

void ErrorList::addError( const QString &str, const QString &filePath  )
{
	if( filePath.isEmpty() )
		mMessages.push_back( Error( true, str, mActiveFilePath ) );
	else
		mMessages.push_back( Error( true, str, filePath ) );
}

ErrorListDialog::ErrorListDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ErrorListDialog)
{
	ui->setupUi(this);
//	ui->errorList->setModel( new QStandardItemModel( this ) );
//	ui->errorList->setIconSize( 32, 32 );
}

void ErrorListDialog::show( const ErrorList &list )
{
	for( QList<ErrorList::Error>::const_iterator msgIt = list.mMessages.begin(); msgIt != list.mMessages.end(); ++msgIt ) {
		QListWidgetItem *item = new QListWidgetItem( ( msgIt->mIsError ) ? QIcon( ":/resources/error.png" ) : QIcon( ":/resources/warning.png" ),
			msgIt->mMsg );
		item->setData( Qt::UserRole, QVariant( msgIt->mFilePath ) );
		ui->errorList->addItem( item );
	}
	setModal( true );
	exec();
}

ErrorListDialog::~ErrorListDialog()
{
	delete ui;
}

void ErrorListDialog::on_okButton_clicked()
{
    accept();
}

void ErrorListDialog::on_errorList_itemClicked(QListWidgetItem *item)
{
    QVariant var = item->data( Qt::UserRole );
    ui->fileInfoLabel->setText( var.value<QString>() );
}
