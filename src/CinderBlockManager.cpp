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

#include "CinderBlockManager.h"
#include "Util.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QUrl>

#include <iostream>
#include <fstream>

CinderBlockManager*	CinderBlockManager::inst()
{
	static CinderBlockManager *sInst = 0;
	if( ! sInst )
		sInst = new CinderBlockManager();
	return sInst;
}

CinderBlockManager::CinderBlockManager()
{
}

void CinderBlockManager::scan( const QString &path, ErrorList *errors )
{
	QDir dir( path );
	dir.setFilter( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
	inst()->scanAndParseCinderBlocks( dir, dir, 2, errors );
}

CinderBlock* CinderBlockManager::findById( const QString &id )
{
	for( QList<CinderBlock>::Iterator bIt = mCinderBlocks.begin(); bIt != mCinderBlocks.end(); ++bIt )
		if( bIt->getId() == id )
			return &(*bIt);
	
	return NULL;
}

void CinderBlockManager::clearInst()
{
	mCinderBlocks.clear();
	mProjectTemplates.clear();
}

void CinderBlockManager::scanAndParseCinderBlocks( const QDir &cinderDir, const QDir &dir, int depth, ErrorList *errorList )
{
	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		if( ( depth > 0 ) && fileInfo.isDir() ) {
			QDir subDir( fileInfo.filePath() );
			subDir.setFilter( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
			scanAndParseCinderBlocks( cinderDir, subDir, depth - 1, errorList );
		}
		else if( fileInfo.fileName() == "cinderblock.xml" ) {
			std::ifstream fs( fileInfo.filePath().toStdString().c_str() );
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load( fs );
			if( result ) {
				pugi::xpath_node_set blocks = doc.select_nodes("/cinder/block");
				for( pugi::xpath_node_set::const_iterator it = blocks.begin(); it != blocks.end(); ++it ) {
					
					QString relative = cinderDir.relativeFilePath( fileInfo.absoluteFilePath() );
					errorList->setActiveFilePath( QString( "<a href=\"" ) + QUrl::fromLocalFile( fileInfo.absoluteFilePath() ).toString() + "\">" + relative + "</a>" );
					
					mCinderBlocks.push_back( CinderBlock( dir.absolutePath(), it->node(), errorList ) );
					
					// does this block have any templates?
					pugi::xpath_node_set templates = doc.select_nodes( "/cinder/template" );
					for( pugi::xpath_node_set::const_iterator it = templates.begin(); it != templates.end(); ++it ) {
						QFileInfo tmplPath( dir, it->node().first_child().value() );
						if( tmplPath.exists() ) {
							try {
								QString relative = cinderDir.relativeFilePath( tmplPath.absoluteFilePath() );
								errorList->setActiveFilePath( QString( "<a href=\"" ) + QUrl::fromLocalFile( tmplPath.absoluteFilePath() ).toString() + "\">" + relative + "</a>" );

								pugi::xml_document doc;
								std::ifstream fs( tmplPath.filePath().toStdString().c_str() );
								pugi::xml_parse_result result = doc.load( fs );
								if( result ) {
									mProjectTemplates.push_back( ProjectTemplate( tmplPath.absoluteDir().absolutePath(),
										doc.select_single_node( "cinder/template" ).node(), errorList ) );
								}
							}
							catch( ... ) {}
						}
					}
				}
				
				QFileInfo iconPath( dir, "cinderblock.png" );
				if( iconPath.exists() ) {
					mIconCache[iconPath.filePath()] = QIcon( iconPath.filePath() );
					mCinderBlocks.back().setIconPath( iconPath.filePath() );
				}
			}
			else {
				//QString msg( "Error parsing CinderBlock:" + dir.relativeFilePath( fileInfo.filePath() ) );
				//msg += QString(result.description());
				errorList->addError( QString(result.description()), fileInfo.absoluteFilePath() );
            }
		}
	}
	
	errorList->setActiveFilePath( "" );
}

const QIcon& CinderBlockManager::getIconInst( const QString &path )
{
	static QIcon defaultIcon( ":/resources/GenericCinderBlock.png" );
	if( path.isEmpty() ) {
		return defaultIcon;
	}
	if( mIconCache.find( path ) != mIconCache.end() )
		mIconCache[path] = QIcon( path );
	
	return mIconCache[path];
}
