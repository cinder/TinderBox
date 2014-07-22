#ifndef ERRORLISTDIALOG_H
#define ERRORLISTDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class ErrorListDialog;
}

class ErrorList {
  public:
	void	addWarning( const QString &str, const QString &filePath = "" );
	void	addError( const QString &str, const QString &filePath = ""  );
	
	void	setActiveFilePath( const QString &activeFilePath ) { mActiveFilePath = activeFilePath; }
	
	void	clear() { mMessages.clear(); }
	bool	empty() const { return mMessages.empty(); }
	
	struct Error {
		Error( bool isError, const QString &msg, const QString &filePath )
			: mIsError( isError ), mMsg( msg ), mFilePath( filePath )
		{}
	
		bool		mIsError; // warning if false
		QString		mMsg;
		QString		mFilePath;
	};
	
	QList<Error>		mMessages;
	QString				mActiveFilePath;
};

class ErrorListDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit ErrorListDialog(QWidget *parent = 0);
	~ErrorListDialog();
	
	void	show( const ErrorList &list );
	
private slots:
	void on_okButton_clicked();
	
	void on_errorList_itemClicked(QListWidgetItem *item);
	
private:
	Ui::ErrorListDialog *ui;
};

#endif // ERRORLISTDIALOG_H
