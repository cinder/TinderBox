#include "TinderBox.h"

#include <QDir>

// Path
QString getAppDirPath();
QString joinPath( const QString &a, const QString &b );
QString joinPathMac( const QString &a, const QString &b );

bool dirExists( const QString &path );
bool fileExists( const QString &path );
bool makePath( const QString &path );
void copyDir( const QString &srcPath, const QString &dstPath, bool overwriteExisting );

// Treats duplicate as a non-failure
void copyFileOrDir( QFileInfo src, QFileInfo dst, bool overwriteExisting, bool replaceContents = false, const QString &replacePrefix = "", bool windowsLineEndings = false );
void copyFile( QFileInfo src, QFileInfo dst, bool replaceContents, QString replacePrefix, bool windowsLineEndings );

std::string toWinPath( const std::string &path );

// Error message handling
void showErrorMsg( const QString &msg, const QString &heading = "" );
bool showConfirmMsg( const QString &msg, const QString &heading = "" );
void showOkMsg( const QString &msg, const QString &heading = "" );

QString loadAndStringReplace( QFileInfo path, QString replacePrefix, QString cinderPath );
