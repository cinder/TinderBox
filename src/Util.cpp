#include "TinderBox.h"
#include "Util.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QUuid>
#include <iostream>

QString getAppDirPath() {
    QString appDirPath = QApplication::applicationDirPath();
#if defined Q_OS_MAC
    QDir appDir( appDirPath );
    appDir.cd( "../../.." );
    appDirPath = appDir.absolutePath();
#else
    QDir appDir( appDirPath );
    // We might need to pop-up a directory if we're in release or debug.
    if( appDirPath.endsWith( "release" ) || appDirPath.endsWith( "debug" ) ) {
        appDir.cd( ".." );
    }
    appDirPath = appDir.absolutePath();
#endif
    return appDirPath;
}

QString joinPath( const QString &a, const QString &b )
{
    return a + QDir::separator() + b;
}

QString joinPathMac( const QString &a, const QString &b )
{
	return a + "/" + b;
}

bool dirExists( const QString &path )
{
    if( path.isEmpty() )
        return false;
    QDir dir( path );
    return dir.exists();
}

bool fileExists( const QString &path )
{
    if( path.isEmpty() )
        return false;
    QFileInfo info( path );
    return info.exists();
}

bool makePath( const QString &path )
{
    QDir dir( path );
    QDir parent( dir );
    parent.cd( ".." );
    return parent.mkpath( dir.dirName() );
}

void copyDir( const QString &srcPath, const QString &dstPath, bool overwriteExisting )
{
	QDir sourceDir( srcPath );
	if( ! sourceDir.exists() )
		throw GenerateFailed( "Couldn't find directory:" + srcPath );

	QDir destDir( dstPath );
	if( ! destDir.exists() ) {
		if( ! QDir::root().mkpath( dstPath ) )
			throw GenerateFailed( "Couldn't create directory:" + dstPath );
	}

	QStringList files = sourceDir.entryList(QDir::Files);
	for( int i = 0; i< files.count(); i++ ) {
		QString srcName = srcPath + QDir::separator() + files[i];
		QString destName = dstPath + QDir::separator() + files[i];
		if( overwriteExisting && QFile::exists( destName ) )
			QFile::remove( destName );
		QFile::copy( srcName, destName );
	}
	files.clear();
	files = sourceDir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
	for( int i = 0; i< files.count(); i++ ) {
		QString srcName = srcPath + QDir::separator() + files[i];
		QString destName = dstPath + QDir::separator() + files[i];
		copyDir( srcName, destName, overwriteExisting );
	}
}

void copyFileOrDir( QFileInfo src, QFileInfo dst, bool overwriteExisting, bool replaceContents, const QString &replacePrefix, bool windowsLineEndings )
{
	// test immediately for the destination already existing. Remove it if we should
	bool dstExists = dst.exists();
	if( dstExists && overwriteExisting ) {
		QFile::remove( dst.absoluteFilePath() );
	}

	QString dstPath;
	QString srcPath = src.absoluteFilePath();

	QDir( QDir::rootPath() ).mkpath( dst.path() );

	dstPath = dst.absoluteFilePath();

	if( src.isDir() )
		copyDir( srcPath, dstPath, overwriteExisting );
	else if( ( ! dstExists ) || overwriteExisting )
		copyFile( QFileInfo( srcPath ), QFileInfo( dstPath ), replaceContents, replacePrefix, windowsLineEndings );
}

void copyFile( QFileInfo src, QFileInfo dst, bool replaceContents, QString replacePrefix, bool windowsLineEndings )
{
	QString dstPath;
	QString srcPath = src.absoluteFilePath();

	QDir( QDir::rootPath() ).mkpath( dst.path() );

	dstPath = dst.absoluteFilePath();

	if( replaceContents ) {
		if( QFileInfo( srcPath ).fileName() == QString( "_TBOX_IGNORE_" ) )
			return;

		QFile srcFile( srcPath );

		if( ! srcFile.open( QFile::ReadOnly | QFile::Text ) ) {
			throw GenerateFailed( "Couldn't open file for reading: " + srcPath );
		}

		QFile dstFile( dstPath );
		if( ! dstFile.open( QFile::WriteOnly ) ) {
			throw GenerateFailed( "Couldn't open file for writing: " + dstPath );
		}

		QTextStream srcStream( &srcFile );
		QTextStream dstStream( &dstFile );

		srcStream.setAutoDetectUnicode( false );
		srcStream.setCodec( "UTF-8" );
		dstStream.setCodec( "UTF-8" );
		dstStream.setGenerateByteOrderMark( false );

		const QString kTBoxPrefix           = "_TBOX_PREFIX_";
		const QString kTBoxProject          = "_TBOX_PROJECT_";
		const QString kUuid0				= "_TBOX_UUID_0_";
		const QString kUuid1				= "_TBOX_UUID_1_";
		const QString kUuid2				= "_TBOX_UUID_2_";
		const QString kLowerUuid0			= "_TBOX_LOWER_UUID_0_";
		const QString kLowerUuid1			= "_TBOX_LOWER_UUID_1_";
		const QString kLowerUuid2			= "_TBOX_LOWER_UUID_2_";

		QString uuids[3];
		uuids[0] = QUuid::createUuid().toString().toUpper().replace( "{", "" ).replace( "}", "" );
		uuids[1] = QUuid::createUuid().toString().toUpper().replace( "{", "" ).replace( "}", "" );;
		uuids[2] = QUuid::createUuid().toString().toUpper().replace( "{", "" ).replace( "}", "" );;
		QString lowerUuids[3];
		lowerUuids[0] = uuids[0].toLower();
		lowerUuids[1] = uuids[1].toLower();
		lowerUuids[2] = uuids[2].toLower();

		while( ! srcStream.atEnd() ) {
			QString line = srcStream.readLine();
			line.replace( kTBoxPrefix, replacePrefix );
			line.replace( kTBoxProject, replacePrefix );

			line.replace( kUuid0, uuids[0] );
			line.replace( kUuid1, uuids[1] );
			line.replace( kUuid2, uuids[2] );
			line.replace( kLowerUuid0, lowerUuids[0] );
			line.replace( kLowerUuid1, lowerUuids[1] );
			line.replace( kLowerUuid2, lowerUuids[2] );

			dstStream << line << ( windowsLineEndings ? "\r\n" : "\n" );
		}
	}
	else {
		if( ! QFile::copy( srcPath, dstPath ) )
			throw GenerateFailed( "Unable to copy from " + srcPath + " to " + dstPath );
	}
}

std::string toWinPath( const std::string &path )
{
	QString temp = QString::fromUtf8( path.c_str() );
	temp.replace( "/", "\\" );
	return temp.toStdString();
}

QString loadAndStringReplace( QFileInfo path, QString replacePrefix, QString cinderPath )
{
	QFile srcFile( path.absoluteFilePath() );
	if( ! srcFile.open( QFile::ReadOnly | QFile::Text ) ) {
		throw GenerateFailed( "Couldn't open file for reading: " + path.absoluteFilePath() );
	}

	QTextStream srcStream( &srcFile );

	srcStream.setAutoDetectUnicode( false );
	srcStream.setCodec( "UTF-8" );

	const QString kTBoxPrefix             = "_TBOX_PREFIX_";
	const QString kTBoxProject            = "_TBOX_PROJECT_";
	const QString kTBoxCinderPathToken    = "_TBOX_CINDER_PATH_";

	QString contents = srcStream.readAll();
	contents.replace( kTBoxPrefix, replacePrefix );
	contents.replace( kTBoxProject, replacePrefix );
	contents.replace( kTBoxCinderPathToken, cinderPath );
	
	return contents;
}

void showErrorMsg( const QString &msg, const QString &heading )
{
    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Critical );
    msgBox.setText( heading );
    msgBox.setInformativeText( msg );
    msgBox.setStandardButtons( QMessageBox::Ok );
    msgBox.setDefaultButton( QMessageBox::Ok );
    msgBox.exec();
}

bool showConfirmMsg( const QString &msg, const QString &heading )
{
    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setText( heading );
    msgBox.setInformativeText( msg );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    int ret = msgBox.exec();
    return QMessageBox::Yes == ret;
}

void showOkMsg( const QString &msg, const QString &heading )
{
    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setText( heading );
    msgBox.setInformativeText( msg );
    msgBox.setStandardButtons( QMessageBox::Ok );
    msgBox.setDefaultButton( QMessageBox::Ok );
    msgBox.exec();
}

