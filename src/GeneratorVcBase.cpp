#include "GeneratorVcBase.h"
#include "ProjectTemplateManager.h"
#include "Util.h"
#include "VcProj.h"

#include <fstream>

GeneratorVcBase::GeneratorVcBase( const QString &foundationName )
	: mFoundationName( foundationName )
{
}

void GeneratorVcBase::setupIncludePaths( VcProjRef proj, Instancer *master, QMap<QString,QString> &conditions, const QString &config, const QString &absPath, const QString &cinderPath )
{
	QList<Template::IncludePath> includePaths = master->getIncludePathsMatchingConditions( conditions );
	for( QList<Template::IncludePath>::ConstIterator pathIt = includePaths.begin(); pathIt != includePaths.end(); ++pathIt )
		proj->addHeaderPath( config, pathIt->getWinOutputPathRelativeTo( absPath, cinderPath ) );
}

void GeneratorVcBase::setupLibraryPaths( VcProjRef proj, Instancer *master, QMap<QString,QString> &conditions, const QString &config, const QString &absPath, const QString &cinderPath )
{
	QList<Template::LibraryPath> libraryPaths = master->getLibraryPathsMatchingConditions( conditions );
	for( QList<Template::LibraryPath>::ConstIterator pathIt = libraryPaths.begin(); pathIt != libraryPaths.end(); ++pathIt )
		proj->addLibraryPath( config, pathIt->getWinOutputPathRelativeTo( absPath, cinderPath ) );
}

void GeneratorVcBase::setupPreprocessorDefines( VcProjRef proj, Instancer *master, QMap<QString,QString> &conditions, const QString &config )
{
	QList<Template::PreprocessorDefine> preprocessorDefines = master->getPreprocessorDefinesMatchingConditions( conditions );
	for( QList<Template::PreprocessorDefine>::ConstIterator defineIt = preprocessorDefines.begin(); defineIt != preprocessorDefines.end(); ++defineIt )
		proj->addPreprocessorDefine( config, defineIt->getValue() );
}

void GeneratorVcBase::generate( Instancer *master )
{
	QMap<QString,QString> conditions = getConditions();
	conditions["config"] = "*";
	QMap<QString,QString> debugConditions = conditions; debugConditions["config"] = "debug";
	QMap<QString,QString> releaseConditions = conditions; releaseConditions["config"] = "release";
	QList<Template::File> files = master->getFilesMatchingConditions( conditions );

	QString absDirPath = master->createDirectory( mFoundationName );
	QString cinderPath = master->getWinRelCinderPath( absDirPath );

	QString replacedVcproj = loadAndStringReplace( ProjectTemplateManager::getFoundationPath( mFoundationName + "/foundation.vcxproj" ),
		master->getNamePrefix(), cinderPath );
	QString replacedVcprojFilters = loadAndStringReplace( ProjectTemplateManager::getFoundationPath( mFoundationName + "/foundation.vcxproj.filters" ),
		master->getNamePrefix(), cinderPath );
	VcProjRef vcProj = createVcProj( replacedVcproj, replacedVcprojFilters );
	vcProj->setupNew( master->getNamePrefix(), getArchitectures(), getSlnDeploy(), getUseRcFile() );

	QList<Template::OutputExtension> outExtensionsDebug = master->getOutputExtensionsMatchingConditions( debugConditions );
	if( ! outExtensionsDebug.empty() )
		vcProj->setTargetExtension( "Debug", "Win32", outExtensionsDebug.first().getValue() );

	QList<Template::OutputExtension> outExtensionsRelease = master->getOutputExtensionsMatchingConditions( releaseConditions );
	if( ! outExtensionsRelease.empty() )
		vcProj->setTargetExtension( "Release", "Win32", outExtensionsRelease.first().getValue() );

	setupIncludePaths( vcProj, master, debugConditions, "Debug", absDirPath, cinderPath );
	setupIncludePaths( vcProj, master, releaseConditions, "Release", absDirPath, cinderPath );

	setupLibraryPaths( vcProj, master, debugConditions, "Debug", absDirPath, cinderPath );
	setupLibraryPaths( vcProj, master, releaseConditions, "Release", absDirPath, cinderPath );

	setupPreprocessorDefines( vcProj, master, debugConditions, "Debug" );
	setupPreprocessorDefines( vcProj, master, releaseConditions, "Release" );

	// setup static libraries
	QList<Template::StaticLibrary> debugStaticLibraries = master->getStaticLibrariesMatchingConditions( debugConditions );
	for( QList<Template::StaticLibrary>::ConstIterator pathIt = debugStaticLibraries.begin(); pathIt != debugStaticLibraries.end(); ++pathIt )
		vcProj->addStaticLibrary( "Debug", pathIt->getWinOutputPathRelativeTo( absDirPath, cinderPath ) );

	QList<Template::StaticLibrary> releaseStaticLibraries = master->getStaticLibrariesMatchingConditions( releaseConditions );
	for( QList<Template::StaticLibrary>::ConstIterator pathIt = releaseStaticLibraries.begin(); pathIt != releaseStaticLibraries.end(); ++pathIt )
		vcProj->addStaticLibrary( "Release", pathIt->getWinOutputPathRelativeTo( absDirPath, cinderPath ) );

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
			if( fileIt->conditionsMatch( debugConditions ) )
				vcProj->addBuildCopy( "Debug", fileIt->getWinOutputPathRelativeTo( absDirPath, cinderPath ) );
			if( fileIt->conditionsMatch( releaseConditions ) )
				vcProj->addBuildCopy( "Release", fileIt->getWinOutputPathRelativeTo( absDirPath, cinderPath ) );
		}
	}

	// write to disk
	vcProj->write( absDirPath, master->getNamePrefix() );
}
