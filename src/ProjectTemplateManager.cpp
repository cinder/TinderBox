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
