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
