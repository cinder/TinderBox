#include "ProjectTemplateManager.h"
#include "Util.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QUrl>

#include <iostream>
#include <fstream>

ProjectTemplateManager*	ProjectTemplateManager::inst()
{
	static ProjectTemplateManager *sInst = 0;
	if( ! sInst )
		sInst = new ProjectTemplateManager();
	return sInst;
}

ProjectTemplateManager::ProjectTemplateManager()
{
}

void ProjectTemplateManager::setCinderDir( QDir cinderDir, ErrorList *errorList )
{
	inst()->mCinderDir = cinderDir;
	inst()->scanImpl( cinderDir, cinderDir.absolutePath() + "/blocks/__AppTemplates", errorList );
}

void ProjectTemplateManager::scanImpl( const QDir &cinderDir, QDir dir, ErrorList *errorList )
{
	if( ! dir.exists() )
		return;
	dir.setFilter( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );

	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		if( fileInfo.isDir() && fileInfo.fileName() != "__Foundation" ) {
			scanImpl( cinderDir, QDir( fileInfo.filePath() ), errorList );
		}
		else if( fileInfo.fileName() == "template.xml" ) {
			std::ifstream fs( fileInfo.filePath().toStdString().c_str() );
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load( fs );

			// set active file path for any reported errors
			QString relative = cinderDir.relativeFilePath( fileInfo.absoluteFilePath() );
			errorList->setActiveFilePath( QString( "<a href=\"" ) + QUrl::fromLocalFile( fileInfo.absoluteFilePath() ).toString() + "\">" + relative + "</a>" );

			if( result ) {
				mTemplates.push_back( ProjectTemplate( dir.absolutePath(), doc.select_single_node( "cinder/template" ).node(), errorList ) );
			}
			else {
				QString msg( "Error parsing template:" + dir.relativeFilePath( fileInfo.filePath() ) );
				//msg += QString(result.description() + QString::fromAscii(" @ ")) + qPrintable(result.offset);
				msg += QString(result.description());
				errorList->addError( msg );
            }
		}
	}
}

QStringList ProjectTemplateManager::getProjectTemplateNamesImpl() const
{
    QStringList result;
	for( QList<ProjectTemplate>::ConstIterator tmplIt = mTemplates.begin(); tmplIt != mTemplates.end(); ++tmplIt )
		result.append( tmplIt->getName() );
	return result;
}

QString	ProjectTemplateManager::getFoundationPathImpl( QString relativePath )
{
	QString templatesPath = joinPath( mCinderDir.absolutePath() + "/blocks/__AppTemplates/__Foundation", relativePath );
    if( ! fileExists( templatesPath ) )
		throw TemplateXmlLoadFailed( "Missing Foundation Template: " + relativePath );

	return templatesPath;
}

const ProjectTemplate&	ProjectTemplateManager::getProjectByIdImpl( const QString &projectId )
{
	for( QList<ProjectTemplate>::const_iterator projIt = mTemplates.begin(); projIt != mTemplates.end(); ++projIt )
		if( projIt->getId() == projectId )
			return *projIt;
			
	throw ProjectNotFoundExc();
}
