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
