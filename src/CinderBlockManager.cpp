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
