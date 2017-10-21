#include "GeneratorXcodeBase.h"
#include "Instancer.h"
#include "Util.h"
#include "XCodeProj.h"
#include "ProjectTemplateManager.h"

#include <QDir>
#include <iostream>
#include <fstream>

QString realXcodeSdkName( const QString &templateXmlName )
{
	if( templateXmlName == "device" )
		return "iphoneos*";
	else if( templateXmlName == "simulator" )
		return "iphonesimulator*";
	return "";
}

QString realConfigName( const QString &templateXmlName )
{
	if( templateXmlName == "debug" )
		return "Debug";
	else if( templateXmlName == "release" )
		return "Release";
	else
		return "";
}

template<typename T>
QList<QString> GeneratorXcodeBase::getUniqueSdks( Instancer *master, QList<T> (Instancer::*fn)( const GeneratorConditions& ) const, const GeneratorConditions &conditions )
{
	QList<QString> sdks = getSdks();
	if( sdks.size() > 1 ) {
		GeneratorConditions conditionsWithSdk = conditions;
		conditionsWithSdk.setCondition( "sdk", sdks[0] );
		QList<T> base = (master->*fn)( conditionsWithSdk );
		bool foundUnequal = false;
		for( int s = 1; s < sdks.size(); ++s ) {
			conditionsWithSdk.setCondition( "sdk", sdks[s] );
			QList<T> thisList = (master->*fn)( conditionsWithSdk );
			if( base != thisList )
				foundUnequal = true;
		}
		
		if( ! foundUnequal ) {
			sdks.clear();
			sdks.push_back( QString( "" ) );
		}
	}
	
	return sdks;
}

void GeneratorXcodeBase::setupIncludePaths( XCodeProjRef xcodeProj, Instancer *master, const GeneratorConditions &conditions,
	const QString &xcodeAbsPath, const QString &cinderPath )
{
	QList<QString> sdks = getUniqueSdks<Template::IncludePath>( master, &Instancer::getIncludePathsMatchingConditions, conditions );
	
	for( QList<QString>::ConstIterator sdkIt = sdks.begin(); sdkIt != sdks.end(); ++sdkIt ) {
		GeneratorConditions conditionsWithSdk = conditions;
		conditionsWithSdk.setCondition( "sdk", *sdkIt );
		QList<Template::IncludePath> includePaths = master->getIncludePathsMatchingConditions( conditionsWithSdk );
		for( QList<Template::IncludePath>::ConstIterator pathIt = includePaths.begin(); pathIt != includePaths.end(); ++pathIt )
			if( pathIt->isSystem() )
				xcodeProj->addSystemHeaderPath( realConfigName( conditions.getConfig() ), realXcodeSdkName( *sdkIt ), pathIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ) );
			else
				xcodeProj->addUserHeaderPath( realConfigName( conditions.getConfig() ), realXcodeSdkName( *sdkIt ), pathIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ) );
	}
}

void GeneratorXcodeBase::setupLibraryPaths( XCodeProjRef xcodeProj, Instancer *master, const GeneratorConditions &conditions,
	const QString &xcodeAbsPath, const QString &cinderPath )
{
	QList<QString> sdks = getUniqueSdks<Template::LibraryPath>( master, &Instancer::getLibraryPathsMatchingConditions, conditions );
	
	for( QList<QString>::ConstIterator sdkIt = sdks.begin(); sdkIt != sdks.end(); ++sdkIt ) {
		GeneratorConditions conditionsWithSdk = conditions;
		conditionsWithSdk.setCondition( "sdk", *sdkIt );
		QList<Template::LibraryPath> includePaths = master->getLibraryPathsMatchingConditions( conditionsWithSdk );
		for( QList<Template::LibraryPath>::ConstIterator pathIt = includePaths.begin(); pathIt != includePaths.end(); ++pathIt )
			xcodeProj->addLibraryPath( realConfigName( conditions.getConfig() ), realXcodeSdkName( *sdkIt ), pathIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ) );
	}
}

void GeneratorXcodeBase::setupFrameworkPaths( XCodeProjRef xcodeProj, Instancer *master, const GeneratorConditions &conditions,
	const QString &xcodeAbsPath, const QString &cinderPath )
{
	QList<QString> sdks = getUniqueSdks<Template::FrameworkPath>( master, &Instancer::getFrameworkPathsMatchingConditions, conditions );

	for( QList<QString>::ConstIterator sdkIt = sdks.begin(); sdkIt != sdks.end(); ++sdkIt ) {
		GeneratorConditions conditionsWithSdk = conditions;
		conditionsWithSdk.setCondition( "sdk", *sdkIt );
		QList<Template::FrameworkPath> frameworkPaths = master->getFrameworkPathsMatchingConditions( conditionsWithSdk );
		for( QList<Template::FrameworkPath>::ConstIterator pathIt = frameworkPaths.begin(); pathIt != frameworkPaths.end(); ++pathIt )
			xcodeProj->addFrameworkPath( realConfigName( conditions.getConfig() ), realXcodeSdkName( *sdkIt ), pathIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ) );
	}
}

void GeneratorXcodeBase::setupStaticLibaries( XCodeProjRef xcodeProj, Instancer *master, const GeneratorConditions &conditions,
	const QString &xcodeAbsPath, const QString &cinderPath )
{
	QList<QString> sdks = getUniqueSdks<Template::StaticLibrary>( master, &Instancer::getStaticLibrariesMatchingConditions, conditions );

	// this is a hack to force usage of both SDKs in iOS for LibraryPaths
	QList<QString> generatorSkds = getSdks();
	for( QList<QString>::ConstIterator sdkIt = generatorSkds.begin(); sdkIt != generatorSkds.end(); ++sdkIt ) {
		if( ! sdks.contains( *sdkIt ) && ( ! sdkIt->isEmpty() ) ) {
			// this is gross, but a non-empty SDK implies empty SDK should be removed
			sdks.removeAll( QString("") );
			sdks.append( *sdkIt );
		}
	}
	
	for( QList<QString>::ConstIterator sdkIt = sdks.begin(); sdkIt != sdks.end(); ++sdkIt ) {
		GeneratorConditions conditionsWithSdk = conditions;
		conditionsWithSdk.setCondition( "sdk", *sdkIt );
		QList<Template::StaticLibrary> libraryPaths = master->getStaticLibrariesMatchingConditions( conditionsWithSdk );
		for( QList<Template::StaticLibrary>::ConstIterator pathIt = libraryPaths.begin(); pathIt != libraryPaths.end(); ++pathIt ) {
			xcodeProj->addStaticLibrary( realConfigName( conditions.getConfig() ), realXcodeSdkName( *sdkIt ), pathIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ) );
		}
	}
}

void GeneratorXcodeBase::setupDynamicLibaries( XCodeProjRef xcodeProj, Instancer *master, const GeneratorConditions &conditions,
	const QString &xcodeAbsPath, const QString &cinderPath )
{
	QList<QString> sdks = getUniqueSdks<Template::DynamicLibrary>( master, &Instancer::getDynamicLibrariesMatchingConditions, conditions );

	for( QList<QString>::ConstIterator sdkIt = sdks.begin(); sdkIt != sdks.end(); ++sdkIt ) {
		GeneratorConditions conditionsWithSdk = conditions;
		conditionsWithSdk.setCondition( "sdk", *sdkIt );
		QList<Template::DynamicLibrary> libraryPaths = master->getDynamicLibrariesMatchingConditions( conditionsWithSdk );
		for( QList<Template::DynamicLibrary>::ConstIterator pathIt = libraryPaths.begin(); pathIt != libraryPaths.end(); ++pathIt ) {
			xcodeProj->addDynamicLibrary( realConfigName( conditions.getConfig() ), realXcodeSdkName( *sdkIt ), pathIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ) );
		}
	}
}

void GeneratorXcodeBase::setupBuildSettings( XCodeProjRef xcodeProj, Instancer *master, const GeneratorConditions &conditions )
{
	QList<QString> sdks = getUniqueSdks<Template::BuildSetting>( master, &Instancer::getBuildSettingsMatchingConditions, conditions );
	
	for( QList<QString>::ConstIterator sdkIt = sdks.begin(); sdkIt != sdks.end(); ++sdkIt ) {
		GeneratorConditions conditionsWithSdk = conditions;
		conditionsWithSdk.setCondition( "sdk", *sdkIt );
		QList<Template::BuildSetting> buildSettings = master->getBuildSettingsMatchingConditions( conditionsWithSdk );
		for( QList<Template::BuildSetting>::ConstIterator settingIt = buildSettings.begin(); settingIt != buildSettings.end(); ++settingIt ) {
			std::cout << "<" << qPrintable(settingIt->getKey()) << "," << qPrintable(settingIt->getValue()) << ">\n";
			if( ! settingIt->getKey().isEmpty() )
				xcodeProj->setBuildSetting( realConfigName( conditions.getConfig() ), *sdkIt, settingIt->getKey(), settingIt->getValue(), true );
		}
	}
}

void GeneratorXcodeBase::setupPreprocessorDefines( XCodeProjRef xcodeProj, Instancer *master, const GeneratorConditions &conditions )
{
	QList<QString> sdks = getUniqueSdks<Template::PreprocessorDefine>( master, &Instancer::getPreprocessorDefinesMatchingConditions, conditions );

	for( QList<QString>::ConstIterator sdkIt = sdks.begin(); sdkIt != sdks.end(); ++sdkIt ) {
		GeneratorConditions conditionsWithSdk = conditions;
		conditionsWithSdk.setCondition( "sdk", *sdkIt );
		QList<Template::PreprocessorDefine> defines = master->getPreprocessorDefinesMatchingConditions( conditionsWithSdk );
		for( QList<Template::PreprocessorDefine>::ConstIterator defineIt = defines.begin(); defineIt != defines.end(); ++defineIt ) {
			xcodeProj->addPreprocessorDefine( realConfigName( conditions.getConfig() ), *sdkIt, defineIt->getValue() );
		}
	}
}

void GeneratorXcodeBase::setupOutputExtension( XCodeProjRef xcodeProj, Instancer *master, const GeneratorConditions &conditions )
{
	QList<QString> sdks = getUniqueSdks<Template::OutputExtension>( master, &Instancer::getOutputExtensionsMatchingConditions, conditions );
	
	for( QList<QString>::ConstIterator sdkIt = sdks.begin(); sdkIt != sdks.end(); ++sdkIt ) {
		GeneratorConditions conditionsWithSdk = conditions;
		conditionsWithSdk.setCondition( "sdk", *sdkIt );
		QList<Template::OutputExtension> outputExtensions = master->getOutputExtensionsMatchingConditions( conditionsWithSdk );
		if( ! outputExtensions.empty() )
			xcodeProj->setBuildSetting( realConfigName( conditions.getConfig() ), *sdkIt, "WRAPPER_EXTENSION", outputExtensions.front().getValue(), true );
	}
}

void GeneratorXcodeBase::generate( Instancer *master )
{
	std::vector<GeneratorConditions> conditions = getConditions();
    QList<Template::File> files = master->getFilesMatchingConditions( conditions );

	// setup output paths of the project itself, and create its parent "xcode" directory
    QString xcodeAbsPath = master->createDirectory( "proj/" + getRootFolderName() );
    QString xcodeprojRelPath = xcodeAbsPath + '/' + master->getNamePrefix() + ".xcodeproj";
    QString xcodeprojAbsPath = master->getAbsolutePath( xcodeprojRelPath );
    QString cinderPath = master->getMacRelCinderPath( xcodeAbsPath );
	master->createDirectory( xcodeprojRelPath );
    QString replaced = loadAndStringReplace( ProjectTemplateManager::getFoundationPath( getRootFolderName() + "/project.pbxproj" ),
            master->getNamePrefix(), cinderPath, xcodeAbsPath );
    XCodeProjRef xcodeProj = XCodeProj::createFromString( replaced );

	// setup include paths
	for( auto condIt = conditions.begin(); condIt != conditions.end(); ++condIt ) {
		setupIncludePaths( xcodeProj, master, *condIt, xcodeAbsPath, cinderPath );
		setupLibraryPaths( xcodeProj, master, *condIt, xcodeAbsPath, cinderPath );
		setupFrameworkPaths( xcodeProj, master, *condIt, xcodeAbsPath, cinderPath );
		setupStaticLibaries( xcodeProj, master, *condIt, xcodeAbsPath, cinderPath );
		setupDynamicLibaries( xcodeProj, master, *condIt, xcodeAbsPath, cinderPath );
	}
	
	// setup files
    for( QList<Template::File>::ConstIterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
		if( fileIt->getType() == Template::File::SOURCE )
			xcodeProj->addSourceFile( fileIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ), fileIt->getVirtualPath(), fileIt->getCompileAs() );
		else if( fileIt->getType() == Template::File::HEADER )
			xcodeProj->addHeaderFile( fileIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ), fileIt->getVirtualPath() );
		else if( fileIt->getType() == Template::File::RESOURCE )
			xcodeProj->addResource( fileIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ),
				fileIt->getVirtualPath(), fileIt->isOutputBuildExcluded() );
		else if( fileIt->getType() == Template::File::FRAMEWORK )
			xcodeProj->addFramework( fileIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ), fileIt->getVirtualPath(),
				fileIt->isOutputAbsolute(), fileIt->isOutputSdkRelative() );
	}

	// we process buildCopy's last so that we can try to reuse an existing fileref
	for( QList<Template::File>::ConstIterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
		if( fileIt->getType() == Template::File::BUILD_COPY )
			xcodeProj->addBuildCopy( fileIt->getMacOutputPathRelativeTo( xcodeAbsPath, cinderPath ), fileIt->getVirtualPath(),
									 fileIt->isOutputAbsolute(), fileIt->isOutputSdkRelative(), fileIt->getBuildCopyDestination() );
	}

	// build settings
	setupBuildSettings( xcodeProj, master, debugConditions, "Debug" );
	setupBuildSettings( xcodeProj, master, releaseConditions, "Release" );

	// preprocessor defines
	setupPreprocessorDefines( xcodeProj, master, debugConditions, "Debug" );
	setupPreprocessorDefines( xcodeProj, master, releaseConditions, "Release" );

	// output extension
	setupOutputExtension( xcodeProj, master, debugConditions, "Debug" );
	setupOutputExtension( xcodeProj, master, releaseConditions, "Release" );

    // write to disk
    QString pbxprojPath = master->getAbsolutePath( xcodeprojRelPath + "/project.pbxproj" );
    std::ofstream fs( pbxprojPath.toUtf8() );
    xcodeProj->print( fs );
}
