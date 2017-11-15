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

#include "Instancer.h"
#include "Preferences.h"
#include "GeneratorBase.h"
#include "Util.h"
#include "Instancer.h"
#include "ProjectTemplateManager.h"

#include <QDir>
#include <QFile>
#include <iostream>
#include <QProcess>

using namespace std;

bool executeGitCommand( const QStringList &params )
{
	std::cout << "Git: ";
	for( QStringList::ConstIterator sIt = params.begin(); sIt != params.end(); ++sIt )
		std::cout << sIt->toStdString() << " ";
	std::cout << std::endl;
	#ifdef Q_OS_MAC
		return QProcess::execute( Preferences::getGitPath(), params ) == 0;
	#else
		return QProcess::execute( "cmd", QStringList() << "/c" << "git" << params ) == 0;
	#endif
}

void Cloner::copyFileOrDir( QFileInfo src, QFileInfo dst, bool overwriteExisting, bool replaceContents, const QString &replacePrefix,
					bool windowsLineEndings )
{
	::copyFileOrDirHelper( src, dst, overwriteExisting, replaceContents, replacePrefix, "" /*replaceProjDir*/, windowsLineEndings );
}

void Cloner::copyFileOrDir( const GeneratorConditions &cond, QFileInfo src, QFileInfo dst, bool overwriteExisting, bool replaceContents, const QString &replacePrefix,
					bool windowsLineEndings )
{
	::copyFileOrDirHelper( src, dst, overwriteExisting, replaceContents, replacePrefix, "" /*replaceProjDir*/, windowsLineEndings );
}

Instancer::Instancer( const ProjectTemplate &projectTmpl )
{
	if( projectTmpl.hasParentProject() ) {
		mChildTemplate = QSharedPointer<ProjectTemplate>( new ProjectTemplate( projectTmpl ) );
		mProjectTmpl = ProjectTemplateManager::getProjectById( projectTmpl.getParentProjectId() );
	}
	else
		mProjectTmpl = projectTmpl;
}

void Instancer::instantiate( bool setupGit )
{
	Cloner cloner;

	if( ! prepareGenerate() )
		return;

	vector<GeneratorConditions> copyConditions;
	for( QList<GeneratorBaseRef>::Iterator childIt = mGenerators.begin(); childIt != mGenerators.end(); ++childIt ) {
		auto childConds = (*childIt)->getConditions();
		copyConditions.insert( copyConditions.end(), childConds.begin(), childConds.end() );
	}

	// set template's output path
	mProjectTmpl.setOutputPath( getOutputDir().absolutePath(), getNamePrefix(), getCinderAbsolutePath() );
	if( mChildTemplate )
		mChildTemplate->setOutputPath( getOutputDir().absolutePath(), getNamePrefix(), getCinderAbsolutePath() );

	// set blocks' output and virtual paths
	for( QList<CinderBlockRef>::Iterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->getInstallType() == CinderBlock::INSTALL_COPY ) {
			const QString outputPath = getOutputDir().absolutePath() + "/blocks/" + (*blockIt)->getName();
			(*blockIt)->setOutputPath( outputPath, getNamePrefix(), getCinderAbsolutePath() );
		}
		else { // install type that doesn't require copying implies output is the same as input
			(*blockIt)->setOutputPathToInput();
		}		
		
		(*blockIt)->setupVirtualPaths( ("Blocks/" + (*blockIt)->getName()) );
	}
	
	// copy any cinderblocks' files
	for( QList<CinderBlockRef>::Iterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->getInstallType() == CinderBlock::INSTALL_COPY )
			(*blockIt)->instantiateFilesMatchingConditions( copyConditions, false, &cloner );
	}

    // get all files which match our generators as well as the empty set of conditions
	mProjectTmpl.instantiateFilesMatchingConditions( copyConditions, false, &cloner );

	if( mChildTemplate )
		mChildTemplate->instantiateFilesMatchingConditions( copyConditions, true, &cloner );

	// bare files (<file> tags) are not the responsibility of the project generators, so the Instancer does them here
	copyBareFiles( copyConditions, &cloner );

	// assets are a special case; we have to copy them all locally since they can't be made relative to the project. Can only live in /assets/
	// do this before we git add
	copyAssets( copyConditions, &cloner );

	// setup git repo and possibly submodules
	if( setupGit ) {
		setupGitRepo( getOutputDir().absolutePath() );

		// do the "git clone"/"git submodule add" dance for any appropriate submodules
		for( QList<CinderBlockRef>::Iterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
			QDir::setCurrent( getOutputDir().absolutePath() );

			if( (*blockIt)->getInstallType() == CinderBlock::INSTALL_GIT_SUBMODULE ) {
				const QString relativeOutputPath = QString::fromUtf8( "blocks" ) + QDir::separator() + (*blockIt)->getName();
				const QString blockParentPath = QDir::toNativeSeparators( (*blockIt)->getParentPath() );
				makePath( getOutputDir().absolutePath() + QDir::separator() + "blocks" ); // Windows needs this to already exist
				if( ! executeGitCommand( QStringList() << "clone" << "--recursive" << blockParentPath << relativeOutputPath ) ) {
					continue;
				}
				if( ! executeGitCommand( QStringList() << "submodule" << "add" << (*blockIt)->getGitUrl() << relativeOutputPath ) ) {
					continue;
				}
				if( ! executeGitCommand( QStringList() << "commit" << "-m" << ("\"Adding " + (*blockIt)->getName() + " submodule\"") ) ) {
					continue;
				}
				QDir::setCurrent( getOutputDir().absolutePath() + QDir::separator() + relativeOutputPath );
				if( ! executeGitCommand( QStringList() << "remote" << "set-url" << "origin" << (*blockIt)->getGitUrl() ) ) {
					continue;
				}
				(*blockIt)->setOutputPath( getOutputDir().absolutePath() + QDir::separator() + relativeOutputPath, getNamePrefix(), getCinderAbsolutePath() );
			}
		}
	}

	// walk the children and generate with each generator
	for( QList<GeneratorBaseRef>::Iterator childIt = mGenerators.begin(); childIt != mGenerators.end(); ++childIt )
		(*childIt)->generate( this );

	// create Resources.h
	writeResourcesHeader( copyConditions );

	if( setupGit ) { // now add it all to the master
		initialCommitToGitRepo( getOutputDir().absolutePath() );
	}
}

void Instancer::copyBareFiles( const vector<GeneratorConditions> &conditions, Cloner *cloner ) const
{
	QList<Template::File> files = getFileTypeMatchingConditions<Template::File::FILE>( conditions, true );

	for( QList<Template::File>::Iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
		cloner->copyFileOrDir( fileIt->getAbsoluteInputPath(), fileIt->getAbsoluteOutputPath(), true, fileIt->getReplaceContents(), getNamePrefix() );
	}
}

void Instancer::copyAssets( const vector<GeneratorConditions> &conditions, Cloner *cloner ) const
{
	// create the assets directory if necessary
	QDir assetDirPath( getOutputDir().absolutePath() + "/assets/" );
	if( ! assetDirPath.exists() ) {
		getOutputDir().mkdir( "assets" );
	}

	QList<Template::File> assets = getFileTypeMatchingConditions<Template::File::ASSET>( conditions, false );

	for( QList<Template::File>::Iterator assetIt = assets.begin(); assetIt != assets.end(); ++assetIt ) {
		// remove a prefix of 'assets' when necessary
		QString relOutputPath = assetIt->getRelativeInputPath();
		if( relOutputPath.indexOf( "assets/") == 0 )
			relOutputPath = relOutputPath.mid( QString("assets/").length() );
		cloner->copyFileOrDir( assetIt->getAbsoluteInputPath(), assetDirPath.absoluteFilePath( relOutputPath ), true, assetIt->getReplaceContents(), getNamePrefix() );
	}
}

void Instancer::writeResourcesHeader( const vector<GeneratorConditions> &conditions ) const
{
	QDir includePath( getOutputDir().absolutePath() + "/include/" );
	if( ! includePath.exists() ) {
		getOutputDir().mkdir( "include" );
	}

	QFileInfo resourcesHeaderPath( getOutputDir().absolutePath() + "/include/Resources.h" );

	QString output;
	if( resourcesHeaderPath.exists() ) { // read the existing file if it exists
		QFile headerFile( resourcesHeaderPath.absoluteFilePath() );
		if( ! headerFile.open( QIODevice::ReadOnly ) )
			throw GenerateFailed( "Failed to load Resources.h header: " + resourcesHeaderPath.absoluteFilePath() );

		QTextStream ts( &headerFile );
		ts.setCodec( "UTF-8" );
		output = ts.readAll();
	}
	else // otherwise just start the file in the normal way
		output += "#include \"cinder/CinderResources.h\"\r\n";

	// get the files matching our conditions
	QList<Template::File> resources = getResourcesMatchingConditions( conditions );

	// iterate all the resources and assign an ID to any which don't have an explicit ID
	for( QList<Template::File>::Iterator curResIt = resources.begin(); curResIt != resources.end(); ++curResIt ) {
		if( curResIt->getResourceId() == -1 ) { // auto-generate resource ID
			bool found;
			int id = 128;
			do {
				found = false;
				for( QList<Template::File>::ConstIterator resIt = resources.begin(); resIt != resources.end(); ++resIt ) {
					if( resIt->getResourceId() == id && resIt->getResourceType() == curResIt->getResourceType() ) {
						++id;
						found = true;
						break;
					}
				}
			} while( found );
			curResIt->setResourceId( id );
		}
	}

	// write the preprocessor defines
	for( QList<Template::File>::ConstIterator fileIt = resources.begin(); fileIt != resources.end(); ++fileIt ) {
		output += "\r\n";
		if( ! fileIt->getResourceName().isEmpty() ) { // we don't add empty-named resources to the header
			// the file path will be relative to the generator ie our_project/xcode, our_project/vc11, etc.
			QFileInfo filePath( fileIt->getMacOutputPathRelativeTo( getOutputDir().absolutePath() + "/generator", getCinderAbsolutePath() ) );
			output += "#define " + fileIt->getResourceName() + "\t\tCINDER_RESOURCE( "
						+ filePath.path() + "/" + ", " + filePath.fileName() + ", "
						+ QString::number( fileIt->getResourceId() ) + ", " + fileIt->getResourceType() + " )";
		}
	}
	output += "\r\n";

	QFile headerFile( resourcesHeaderPath.absoluteFilePath() );
	if( ! headerFile.open( QIODevice::WriteOnly ) )
		throw GenerateFailed( "Failed to write Resources header: " + resourcesHeaderPath.absoluteFilePath() );

	QTextStream ts( &headerFile );
	ts.setCodec( "UTF-8" );
	ts << output;
}

// 'getCopyOnly' returns only the files of copied CinderBlocks; appropriate for <asset> and <file>
template<Template::File::Type FILE_TYPE>
QList<Template::File> Instancer::getFileTypeMatchingConditions( const vector<GeneratorConditions> &copyConditions, bool getCopyOnly ) const
{
	QList<Template::File> allFiles;
	
	allFiles.append( mProjectTmpl.getFilesMatchingConditions( copyConditions ) );
	if( mChildTemplate ) {
		QList<Template::File> childFiles = mChildTemplate->getFilesMatchingConditions( copyConditions );
		// iterate any files we received from the master template and override them with any equivalents from the child template
		for( QList<Template::File>::ConstIterator childFile = childFiles.begin(); childFile != childFiles.end(); ++childFile ) {
			for( QList<Template::File>::Iterator testFile = allFiles.begin(); testFile != allFiles.end(); ++testFile ) {
				if( testFile->getAbsoluteOutputPath() == childFile->getAbsoluteOutputPath() ) {
					allFiles.erase( testFile );
					break;
				}
			}
		}
		allFiles.append( childFiles );
	}
	
	// iterate blocks
	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( getCopyOnly && ( (*blockIt)->getInstallType() != CinderBlock::INSTALL_COPY ) )
			continue;
		for( auto condIt = copyConditions.begin(); condIt != copyConditions.end(); ++condIt ) {
			if( (*blockIt)->supportsConditions( *condIt ) ) {
				allFiles.append( (*blockIt)->getFilesMatchingConditions( *condIt ) );
				break;
			}
		}
	}

	// remove everything that isn't of type FILE_TYPE
	QList<Template::File> result;
	for( QList<Template::File>::Iterator fileIt = allFiles.begin(); fileIt != allFiles.end(); ++fileIt )
		if( fileIt->getType() == FILE_TYPE )
			result.push_back( *fileIt );

	return result;
}

QList<Template::File> Instancer::getResourcesMatchingConditions( const vector<GeneratorConditions> &conditions ) const
{
	return getFileTypeMatchingConditions<Template::File::RESOURCE>( conditions, false );
}

QList<Template::File> Instancer::getFilesMatchingConditions( const GeneratorConditions &conditions ) const
{
	QList<Template::File> result;
	result.append( mProjectTmpl.getFilesMatchingConditions( conditions ) );
	if( mChildTemplate ) {
		QList<Template::File> childFiles = mChildTemplate->getFilesMatchingConditions( conditions );
		// iterate any files we received from the master template and override them with any equivalents from the child template
		for( QList<Template::File>::ConstIterator childFile = childFiles.begin(); childFile != childFiles.end(); ++childFile ) {
			for( QList<Template::File>::Iterator testFile = result.begin(); testFile != result.end(); ++testFile ) {
				if( testFile->getAbsoluteOutputPath() == childFile->getAbsoluteOutputPath() ) {
					result.erase( testFile );
					break;
				}
			}
		}
		result.append( childFiles );
	}

	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->supportsConditions( conditions ) )
			result.append( (*blockIt)->getFilesMatchingConditions( conditions ) );
	}

	return result;
}

QList<Template::IncludePath> Instancer::getIncludePathsMatchingConditions( const GeneratorConditions &conditions ) const
{
	QList<Template::IncludePath> result;
	result.append( mProjectTmpl.getIncludePathsMatchingConditions( conditions ) );
	if( mChildTemplate )
		result.append( mChildTemplate->getIncludePathsMatchingConditions( conditions ) );
	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->supportsConditions( conditions ) )
			result.append( (*blockIt)->getIncludePathsMatchingConditions( conditions ) );
	}

	return result;
}

QList<Template::LibraryPath> Instancer::getLibraryPathsMatchingConditions( const GeneratorConditions &conditions ) const
{
	QList<Template::LibraryPath> result;
	result.append( mProjectTmpl.getLibraryPathsMatchingConditions( conditions ) );
	if( mChildTemplate )
		result.append( mChildTemplate->getLibraryPathsMatchingConditions( conditions ) );
	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->supportsConditions( conditions ) )
			result.append( (*blockIt)->getLibraryPathsMatchingConditions( conditions ) );
	}

	return result;
}

QList<Template::FrameworkPath> Instancer::getFrameworkPathsMatchingConditions( const GeneratorConditions &conditions ) const
{
	QList<Template::FrameworkPath> result;
	result.append( mProjectTmpl.getFrameworkPathsMatchingConditions( conditions ) );
	if( mChildTemplate )
		result.append( mChildTemplate->getFrameworkPathsMatchingConditions( conditions ) );
	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->supportsConditions( conditions ) )
			result.append( (*blockIt)->getFrameworkPathsMatchingConditions( conditions ) );
	}

	return result;
}

QList<Template::StaticLibrary> Instancer::getStaticLibrariesMatchingConditions( const GeneratorConditions &conditions ) const
{
	QList<Template::StaticLibrary> result;
	result.append( mProjectTmpl.getStaticLibrariesMatchingConditions( conditions ) );
	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->supportsConditions( conditions ) )
			result.append( (*blockIt)->getStaticLibrariesMatchingConditions( conditions ) );
		}

	return result;
}

QList<Template::DynamicLibrary> Instancer::getDynamicLibrariesMatchingConditions( const GeneratorConditions &conditions ) const
{
	QList<Template::DynamicLibrary> result;
	result.append( mProjectTmpl.getDynamicLibrariesMatchingConditions( conditions ) );
	if( mChildTemplate )
		result.append( mChildTemplate->getDynamicLibrariesMatchingConditions( conditions ) );
	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->supportsConditions( conditions ) )
			result.append( (*blockIt)->getDynamicLibrariesMatchingConditions( conditions ) );
		}

	return result;
}

QList<Template::BuildSetting> Instancer::getBuildSettingsMatchingConditions( const GeneratorConditions &conditions ) const
{
	QList<Template::BuildSetting> result;
	result.append( mProjectTmpl.getBuildSettingsMatchingConditions( conditions ) );
	if( mChildTemplate )
		result.append( mChildTemplate->getBuildSettingsMatchingConditions( conditions ) );
	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->supportsConditions( conditions ) )
			result.append( (*blockIt)->getBuildSettingsMatchingConditions( conditions ) );
	}

	return result;
}

QList<Template::PreprocessorDefine> Instancer::getPreprocessorDefinesMatchingConditions( const GeneratorConditions &conditions ) const
{
	QList<Template::PreprocessorDefine> result;
	result.append( mProjectTmpl.getPreprocessorDefinesMatchingConditions( conditions ) );
	if( mChildTemplate )
		result.append( mChildTemplate->getPreprocessorDefinesMatchingConditions( conditions ) );
	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->supportsConditions( conditions ) )
			result.append( (*blockIt)->getPreprocessorDefinesMatchingConditions( conditions ) );
	}

	return result;
}

QList<Template::OutputExtension> Instancer::getOutputExtensionsMatchingConditions( const GeneratorConditions &conditions ) const
{
	QList<Template::OutputExtension> result;
	result.append( mProjectTmpl.getOutputExtensionsMatchingConditions( conditions ) );
	if( mChildTemplate )
		result.append( mChildTemplate->getOutputExtensionsMatchingConditions( conditions ) );
	for( QList<CinderBlockRef>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( (*blockIt)->supportsConditions( conditions ) )
			result.append( (*blockIt)->getOutputExtensionsMatchingConditions( conditions ) );
	}

	return result;
}

void Instancer::addGenerator( GeneratorBase *childGen )
{
	mGenerators.push_back( QSharedPointer<GeneratorBase>( childGen ) );
}

// Creates the destination directory, sets Cinder location
bool Instancer::prepareGenerate()
{
	QString cinderLocation = getCinderAbsolutePath();
	if( ! dirExists( cinderLocation )  ) {
		throw GenerateFailed( "Cinder location is invalid or does not exist: " + cinderLocation );
	}

	if( ! dirExists( getBaseLocation() ) ) {
		QString msg = "The directory \"" + getBaseLocation() + "\" doesn't exist. Should TinderBox create it?";
		QString heading = "Confirm Directory Creation";
		if( showConfirmMsg( msg, heading ) ) {
			if( ! makePath( getBaseLocation() ) ) {
				throw GenerateFailed( "Couldn't create directory: " + getBaseLocation() );
			}
		}
		else {
			return false;
		}
	}

	QString destinationPath = joinPath( getBaseLocation(), getProjectName() );
	QDir destinationDir( destinationPath );
	if( destinationDir.exists() ) {
		throw GenerateFailed( destinationPath + " already exists! Please specify a different Name Prefix." );
	}

	if( ! makePath( destinationPath ) ) {
		throw GenerateFailed( "Couldn't create Project directory: " + destinationPath );
	}

	// setup Cinder relative path
	mAbsCinderPath = cinderLocation;

	return true;
}

bool Instancer::setupGitRepo( const QString &dirPath )
{
	QDir::setCurrent( dirPath );
	return executeGitCommand( QStringList() << "init" );
}

bool Instancer::initialCommitToGitRepo( const QString &dirPath )
{
	QDir::setCurrent( dirPath );
	if( ! executeGitCommand( QStringList() << "add" << "." ) )
		return false;
	if( ! executeGitCommand( QStringList() << "commit" << "-m" << "\"Initial commit\"" ) )
		return false;
	return true;
}

QString Instancer::getProjectName() const
{
	return mProjectName;
}

void Instancer::setProjectName( const QString &projName )
{
	mProjectName = projName ;
}

QString Instancer::getNamePrefix() const
{
	return mNamePrefix;
}

void Instancer::setNamePrefix( const QString &namePrefix )
{
	mNamePrefix = namePrefix;
}

QString Instancer::getBaseLocation() const
{
	return mBaseLocation;
}

void Instancer::setBaseLocation( const QString &baseLocation )
{
	mBaseLocation = baseLocation;
}

QString Instancer::getMacRelCinderPath( const QString &relativeTo ) const
{
	QString result = getRelCinderPath( relativeTo );
	// if the path is inside the Cinder folder (like /samples) we need to strip the trailing '/'
	if( result.endsWith( '/' ) )
		result.truncate( result.length() - 1 );
	return result;
}

void Instancer::setCinderAbsolutePath( const QString &cinderLocation )
{
	mAbsCinderPath = cinderLocation;
}

QString Instancer::getRelCinderPath( const QString &relativeTo ) const
{
	QDir dir( relativeTo );
	auto result = dir.relativeFilePath( mAbsCinderPath );
	if( result.endsWith('/') )
		result.chop( 1 );
	return result;
}

QString Instancer::createDirectory( QString relPath ) const
{
	getOutputDir().mkpath( relPath );
	return getOutputDir().absoluteFilePath( relPath );
}

QString	Instancer::getAbsolutePath( QString relPath ) const
{
	return getOutputDir().absoluteFilePath( relPath );
}
