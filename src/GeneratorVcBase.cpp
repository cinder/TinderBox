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

#include "GeneratorVcBase.h"
#include "ProjectTemplateManager.h"
#include "Util.h"
#include "VcProj.h"

#include <fstream>

GeneratorVcBase::GeneratorVcBase( const QString &foundationName )
	: mFoundationName( foundationName )
{
}

void GeneratorVcBase::setupIncludePaths( VcProjRef proj, Instancer *master, const VcProj::ProjectConfiguration &config, const QString &absPath, const QString &cinderPath )
{
	QList<Template::IncludePath> includePaths = master->getIncludePathsMatchingConditions( config.getConditions() );
	for( QList<Template::IncludePath>::ConstIterator pathIt = includePaths.begin(); pathIt != includePaths.end(); ++pathIt )
		proj->addHeaderPath( config, pathIt->getWinOutputPathRelativeTo( absPath, cinderPath ) );
}

void GeneratorVcBase::setupLibraryPaths( VcProjRef proj, Instancer *master, const VcProj::ProjectConfiguration &config, const QString &absPath, const QString &cinderPath )
{
	QList<Template::LibraryPath> libraryPaths = master->getLibraryPathsMatchingConditions( config.getConditions() );
	for( QList<Template::LibraryPath>::ConstIterator pathIt = libraryPaths.begin(); pathIt != libraryPaths.end(); ++pathIt )
		proj->addLibraryPath( config, pathIt->getWinOutputPathRelativeTo( absPath, cinderPath ) );
}

void GeneratorVcBase::setupPreprocessorDefines( VcProjRef proj, Instancer *master, const VcProj::ProjectConfiguration &config )
{
	QList<Template::PreprocessorDefine> preprocessorDefines = master->getPreprocessorDefinesMatchingConditions( config.getConditions() );
	for( QList<Template::PreprocessorDefine>::ConstIterator defineIt = preprocessorDefines.begin(); defineIt != preprocessorDefines.end(); ++defineIt )
		proj->addPreprocessorDefine( config, defineIt->getValue() );
}

void GeneratorVcBase::generate( Instancer *master )
{
	QMap<QString,QString> conditions = getConditions();
	conditions["config"] = "*";
	QList<Template::File> files = master->getFilesMatchingConditions( conditions );

	auto projectConfigurations = getPlatformConfigurations();

	QString absDirPath = master->createDirectory( mFoundationName );
	QString cinderPath = master->getWinRelCinderPath( absDirPath );

    // Load the foundation .vcxproj and .filters files as strings; replace _TBOX_CINDER_PATH_ appropriately
	QString replacedVcproj = loadAndStringReplace( ProjectTemplateManager::getFoundationPath( mFoundationName + "/foundation.vcxproj" ),
        master->getNamePrefix(), cinderPath, absDirPath );
	QString replacedVcprojFilters = loadAndStringReplace( ProjectTemplateManager::getFoundationPath( mFoundationName + "/foundation.vcxproj.filters" ),
        master->getNamePrefix(), cinderPath, absDirPath );
    // parse these strings into an instance of VcProj
    VcProjRef vcProj = createVcProj( replacedVcproj, replacedVcprojFilters );
    vcProj->setupNew( master->getNamePrefix(), getPlatformConfigurations(), getSlnDeploy(), getUseRcFile() );

	for( const auto &config : projectConfigurations ) {
		QList<Template::OutputExtension> outExtensions = master->getOutputExtensionsMatchingConditions( config.getConditions() );
		if( ! outExtensions.empty() )
			vcProj->setTargetExtension( config, outExtensions.first().getValue() );

		setupIncludePaths( vcProj, master, config, absDirPath, cinderPath );
		setupLibraryPaths( vcProj, master, config, absDirPath, cinderPath );
		setupPreprocessorDefines( vcProj, master, config );

		// setup static libraries
		QList<Template::StaticLibrary> staticLibraries = master->getStaticLibrariesMatchingConditions( config.getConditions() );
		for( QList<Template::StaticLibrary>::ConstIterator pathIt = staticLibraries.begin(); pathIt != staticLibraries.end(); ++pathIt )
			vcProj->addStaticLibrary( config, pathIt->getWinOutputPathRelativeTo( absDirPath, cinderPath ) );
	}

	// There is nothing to do for Dynamic libraries in VC so just ignore them.

	// Iterate the Files.
	for( QList<Template::File>::ConstIterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
		if( fileIt->getType() == Template::File::SOURCE )
			vcProj->addSourceFile( fileIt->getWinOutputPathRelativeTo( absDirPath, cinderPath ), fileIt->getVirtualPath() );
		else if( fileIt->getType() == Template::File::HEADER )
			vcProj->addHeaderFile( fileIt->getWinOutputPathRelativeTo( absDirPath, cinderPath ), fileIt->getVirtualPath(), fileIt->isResourceHeader() );
		else if( fileIt->getType() == Template::File::RESOURCE )
			vcProj->addResourceFile( fileIt->getResourceName(), fileIt->getWinOutputPathRelativeTo( absDirPath, cinderPath ), fileIt->getResourceType(), fileIt->getResourceId() );
		else if( fileIt->getType() == Template::File::BUILD_COPY ) {
			for( const auto &config : projectConfigurations ) {
				if( fileIt->conditionsMatch( config.getConditions() ) )
					vcProj->addBuildCopy( config, fileIt->getWinOutputPathRelativeTo( absDirPath, cinderPath ) );
			}
		}
	}

	// write to disk
	vcProj->write( absDirPath, master->getNamePrefix() );
}
