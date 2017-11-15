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
void copyFileOrDirHelper( QFileInfo src, QFileInfo dst, bool overwriteExisting, bool replaceContents = false, const QString &replacePrefix = "",
					const QString &replaceProjDir = "", bool windowsLineEndings = false );
void copyFileHelper( QFileInfo src, QFileInfo dst, bool replaceContents, QString replacePrefix, QString replaceProjDir, bool windowsLineEndings );

std::string toWinPath( const std::string &path );

// Error message handling
void showErrorMsg( const QString &msg, const QString &heading = "" );
bool showConfirmMsg( const QString &msg, const QString &heading = "" );
void showOkMsg( const QString &msg, const QString &heading = "" );

QString loadAndStringReplace( QFileInfo path, QString replacePrefix, QString cinderPath, QString replaceProjectDir );
