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

#include "Template.h"
#include "Util.h"

#include <iostream>

bool attributeEquals( const pugi::xml_node &node, const std::string &attrName, const std::string &compareValue, bool defaultResult = false )
{
	pugi::xml_attribute_iterator attIt = node.attributes_begin();
	while( attIt != node.attributes_end() ) {
		if( QString::fromUtf8( attIt->name() ).toLower() == QString::fromUtf8( attrName.c_str() ).toLower() ) {
			return( QString::fromUtf8( attIt->value()).toLower() == QString::fromUtf8( compareValue.c_str() ).toLower() );
		}
		++attIt;
	}

	return defaultResult;
}

void testKnownAttributes( const pugi::xml_node &node, const QStringList &knownAttributes, const QString &tag, ErrorList *resultErrors )
{
	pugi::xml_attribute_iterator attIt = node.attributes_begin();
	while( attIt != node.attributes_end() ) {
		bool found = false;
		for( QStringList::ConstIterator knownIt = knownAttributes.constBegin(); knownIt != knownAttributes.constEnd(); ++knownIt ) {
			if( QString::fromUtf8( attIt->name() ).toLower() == knownIt->toLower() ) {
				found = true;
				break;
			}
		}
		if( ! found )
			resultErrors->addWarning( "Ignoring unknown attribute \"" + QString::fromUtf8( attIt->name() ) + "\" on <" + tag + "> tag." );
		++attIt;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::Item
Template::Item::Item( const QString &parentPath, const QString &inputPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions )
	: mConditions( conditions )
{
	mInputRelativePath = inputPath;
	mInputAbsolutePath = QDir( parentPath ).absoluteFilePath( mInputRelativePath );
	mOutputAbsolutePath = "";
	mOutputIsAbsolute = attributeEquals( dom, "absolute", "true" );
	mOutputIsSdkRelative = attributeEquals( dom, "sdk", "true" );
	mOutputIsCinderRelative = attributeEquals( dom, "cinder", "true" );
	mBuildExclude = attributeEquals( dom, "buildexclude", "true" );
	if( mOutputIsSdkRelative ) {
		mOutputIsAbsolute = true;
		mInputRelativePath = "System/Library/Frameworks/" + mInputRelativePath;
	}
}

bool Template::Item::conditionsMatch( const QMap<QString,QString> &conditions ) const
{
	for( QMap<QString,QString>::ConstIterator condIt = mConditions.begin(); condIt != mConditions.end(); ++condIt ) {
		const QString key = condIt.key();
		const QString val = condIt.value();
		if( ( ! conditions.contains( condIt.key() ) ) || ( (conditions[condIt.key()] != condIt.value()) && ( conditions[condIt.key()] != "*") ) )
			return false;
	}

	return true;
}

QStringList	Template::Item::knownAttributes()
{
	return QStringList() << "absolute" << "sdk" << "cinder" << "buildexclude";
}

QString	Template::Item::getAbsoluteOutputPath() const
{
	return mOutputAbsolutePath;
}

void Template::Item::setOutputPath( const QString &outputPath, const QString &/*replaceName*/, const QString &cinderPath )
{
	QString replacedName = mInputRelativePath;

	if( outputPath.isEmpty() ) {
		mOutputAbsolutePath = mInputAbsolutePath;
	}
	else if( mOutputIsCinderRelative ) {
		QDir cinder( cinderPath );
		mOutputAbsolutePath = cinder.filePath( mInputRelativePath );
	}
	else if( mOutputIsAbsolute || mOutputIsSdkRelative )
		mOutputAbsolutePath = mInputRelativePath;
	else {
		QDir dir( outputPath );
		mOutputAbsolutePath = dir.absoluteFilePath( replacedName );
	}	
}

QString Template::Item::getOutputPathRelativeTo( const QString &relativeTo, const QString &cinderPath ) const
{
	if( mOutputIsCinderRelative ) {
		QDir cinder( cinderPath );
		return cinder.filePath( mInputRelativePath );
	}
	else if( mOutputIsAbsolute || mOutputIsSdkRelative )
		return mInputRelativePath;
	else {
		QDir dir( relativeTo );
		return dir.relativeFilePath( mOutputAbsolutePath );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::IncludePath
Template::IncludePath::IncludePath( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors )
	: Item( parentPath, QString::fromUtf8( dom.first_child().value() ), dom, conditions )
{
	mSystem = attributeEquals( dom, "system", "true" );

	testKnownAttributes( dom, Item::knownAttributes() << "system", "includepath", errors );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::LibraryPath
Template::LibraryPath::LibraryPath( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors )
	: Item( parentPath, QString::fromUtf8( dom.first_child().value() ), dom, conditions )
{
	testKnownAttributes( dom, Item::knownAttributes(), "librarypath", errors );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::FrameworkPath
Template::FrameworkPath::FrameworkPath( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors )
	: Item( parentPath, QString::fromUtf8( dom.first_child().value() ), dom, conditions )
{
	testKnownAttributes( dom, Item::knownAttributes(), "frameworkpath", errors );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::File
Template::File::File( const QString &parentPath, const QString &inputPath, const pugi::xml_node &dom, Type type, const QMap<QString,QString> &conditions, ErrorList *errors )
	: Item( parentPath, inputPath, dom, conditions ), mType( type ), mResourceHeader( false ), mPch( false )
{
	mReplaceContents = attributeEquals( dom, "replaceContents", "true" );
	mReplaceName = attributeEquals( dom, "replaceName", "true" );
	// include-specific
	mPch = attributeEquals( dom, "isPCH", "true" );
	mResourceHeader = attributeEquals( dom, "isResourceHeader", "true" );
	// resource-specific
	mResourceName = QString::fromUtf8( dom.attribute("name").value() );
	mResourceType = QString::fromUtf8( dom.attribute("type").value() );
	QString resourceIdString = QString::fromUtf8( dom.attribute("id").value() );
	if( resourceIdString.isEmpty() || resourceIdString.toLower() == "auto" )
		mResourceId = -1;
	else
		mResourceId = resourceIdString.toInt();
	mBuildCopyDestination = QString::fromUtf8( dom.attribute("destination").value() );
	if( mBuildCopyDestination.toLower() != "frameworks" && mBuildCopyDestination.toLower() != "executables" && mBuildCopyDestination.toLower() != "plugins" && ( ! mBuildCopyDestination.isEmpty() ) )
		errors->addWarning( "Unknown value for destination attribute \"" + mBuildCopyDestination + "\"." );

	mCompileAs = QString::fromUtf8( dom.attribute("compileAs").value() ).toLower();
	if( mCompileAs.isEmpty() )
		mCompileAs = QFileInfo( mInputRelativePath ).suffix();
	if( mOutputIsCinderRelative )
		mVirtualPath = "Cinder/" + mInputRelativePath;
	else
		mVirtualPath = "";
	
	QString tagName = QString::fromUtf8( dom.name() ).toLower();
	testKnownAttributes( dom, Item::knownAttributes() << "replacecontents" << "replacename" << "compileas" << "ispch" << "isresourceheader" << "name" << "type" << "id" << "copy" << "destination", tagName, errors );
	if( ( mType != File::HEADER ) && mPch )
		errors->addWarning( "Non-header marked as PCH \"" + inputPath + "\"." );
	if( ( mType != File::BUILD_COPY ) && ( ! mBuildCopyDestination.isEmpty() ) )
		errors->addWarning( "Non-build copy has \"destination\" attribute." );
	if( ( mType != File::HEADER ) && mResourceHeader )
		errors->addWarning( "Non-header marked as Resources header \"" + inputPath + "." );
	if( ( conditions.find( "sdk" ) != conditions.end() ) && ( ! conditions["sdk"].isEmpty() ) )
		errors->addWarning( "Ignoring \"sdk\" on " + inputPath + "." );
}

void Template::File::setInputPath( const QString &parentPath, const QString &inputPath )
{
	mInputRelativePath = inputPath;
	mInputAbsolutePath = QDir( parentPath ).absoluteFilePath( mInputRelativePath );
	if( mOutputIsSdkRelative ) {
		mInputRelativePath = "System/Library/Frameworks/" + mInputRelativePath;
	}

	if( mOutputIsCinderRelative )
		mVirtualPath = "Cinder/" + mInputRelativePath;
}

void Template::File::setOutputPath( const QString &outputPath, const QString &replaceName, const QString &cinderPath )
{
	QString replacedName = mInputRelativePath;
	if( mReplaceName ) {
		replacedName.replace( "_TBOX_PREFIX_", replaceName );
	}

	if( outputPath.isEmpty() ) {
		mOutputAbsolutePath = mInputAbsolutePath;
	}
	else if( mOutputIsCinderRelative ) {
		QDir cinder( cinderPath );
		mOutputAbsolutePath = cinder.filePath( mInputRelativePath );
	}
	else if( mOutputIsAbsolute || mOutputIsSdkRelative )
		mOutputAbsolutePath = mInputRelativePath;
	else {
		QDir dir( outputPath );
		mOutputAbsolutePath = dir.absoluteFilePath( replacedName );
	}
}

QString	Template::File::getMacOutputPath( const QString &outputPath, const QString &replacePrefix, const QString &cinderPath ) const
{
	QString replacedName = mInputRelativePath;
	if( mReplaceName ) {
		replacedName.replace( "_TBOX_PREFIX_", replacePrefix );
	}

	if( mOutputIsCinderRelative ) {
		QDir cinder( cinderPath );
		return cinder.filePath( replacedName );
	}
	else if( mOutputIsAbsolute || mOutputIsSdkRelative )
		return replacedName;
	else {
		QDir dir( outputPath );
		return dir.absoluteFilePath( replacedName );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::StaticLibrary
Template::StaticLibrary::StaticLibrary( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors )
	: Item( parentPath, QString::fromUtf8( dom.first_child().value() ), dom, conditions )
{
	testKnownAttributes( dom, Item::knownAttributes(), "staticlibrary", errors );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::DynamicLibrary
Template::DynamicLibrary::DynamicLibrary( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors )
	: Item( parentPath, QString::fromUtf8( dom.first_child().value() ), dom, conditions )
{
	testKnownAttributes( dom, Item::knownAttributes(), "dynamiclibrary", errors );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::BuildSetting
Template::BuildSetting::BuildSetting( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors )
	: Item( parentPath, QString::fromUtf8( dom.first_child().value() ), dom, conditions )
{
	mKey = QString::fromUtf8( dom.attribute("name").value() );
	mValue = QString::fromUtf8( dom.first_child().value() );
	
	testKnownAttributes( dom, QStringList() << "name", "setting", errors );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::PreprocessorDefine
Template::PreprocessorDefine::PreprocessorDefine( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors )
	: Item( parentPath, QString::fromUtf8( dom.first_child().value() ), dom, conditions )
{
	Q_UNUSED(errors);
	mValue = QString::fromUtf8( dom.first_child().value() );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::OutputExtension
Template::OutputExtension::OutputExtension( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors )
	: Item( parentPath, QString::fromUtf8( dom.first_child().value() ), dom, conditions )
{
	Q_UNUSED(errors);
	mValue = QString::fromUtf8( dom.first_child().value() );
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Template::Template
Template::Template( const QString &parentPath, const pugi::xml_node &doc, ErrorList *errors )
	: mParentPath( parentPath ), mCore( false )
{
	QMap<QString,QString> emptyConditions;
	mId = QString::fromUtf8( doc.attribute( "id" ).value() );
	parseGroup( doc, emptyConditions, errors );
}

// Converts a <sourcePattern> or <headerPattern> into a group of <source> or <header> files
void Template::processFilePattern( const QString &parentPath, const pugi::xml_node &dom, File::Type type, const QMap<QString,QString> &conditions, ErrorList *errors )
{
	// Create a temporary Template::File to extract the attributes and data
	Template::File tempFile( parentPath, QString::fromUtf8( dom.first_child().value() ), dom, type, conditions, errors );

	QList<File> result;
	QString pathPart = QFileInfo( tempFile.getRelativeInputPath() ).path(); // Ex: Box2D/src/Box2D/Rope
	QString namePart = QFileInfo( tempFile.getRelativeInputPath() ).fileName(); // Ex: *.cpp

	QDir dir( QDir( parentPath ).absoluteFilePath( pathPart ) );
	// use the namePart as a regex-style filter
	dir.setNameFilters( QStringList() << namePart );
	dir.setFilter( QDir::NoDotAndDotDot | QDir::Files );
	QStringList entries = dir.entryList();
	for( QStringList::Iterator sIt = entries.begin(); sIt != entries.end(); ++sIt ) {
		mFiles.push_back( tempFile );
		mFiles.back().setInputPath( parentPath, QDir( pathPart ).filePath( *sIt ) );
	}
}

void Template::parseGroup( const pugi::xml_node &node, QMap<QString,QString> conditions, ErrorList *errors )
{
	QString tagName = QString::fromUtf8( node.name() ).toLower();

	if( tagName == "template" || tagName == "block" ) {
		mName = node.attribute( "name" ).value();
		mCore = attributeEquals( node, "core", "true" );
	}
	// with a platform node we need to parse compiler & os
	else if( tagName == QString("platform") ) {
		QString compiler = QString::fromUtf8( node.attribute("compiler").value() ).toLower();
		QString os = QString::fromUtf8( node.attribute("os").value() ).toLower();
		QString config = QString::fromUtf8( node.attribute("config").value() ).toLower();
		QString sdk = QString::fromUtf8( node.attribute("sdk").value() ).toLower();
		if( ! compiler.isEmpty() )
			conditions["compiler"] = compiler;
		if( ! os.isEmpty() )
			conditions["os"] = os;
		if( ! config.isEmpty() )
			conditions["config"] = config;
		if( ! sdk.isEmpty() )
			conditions["sdk"] = sdk;
			
		testKnownAttributes( node, QStringList() << "compiler" << "os" << "config" << "sdk", "platform", errors );				
	}

	// iterate the children
	pugi::xml_node targetNode = node.first_child();
	while( targetNode ) {
		QString tagName = QString::fromUtf8( targetNode.name() ).toLower();
		if( tagName == "source" ) {
			mFiles.push_back( File( mParentPath, QString::fromUtf8( targetNode.first_child().value() ), targetNode, File::SOURCE, conditions, errors ) );
		}
		else if( tagName == "sourcepattern" ) {
			processFilePattern( mParentPath, targetNode, File::SOURCE, conditions, errors );
		}
		else if( tagName == "header" ) {
			mFiles.push_back( File( mParentPath, QString::fromUtf8( targetNode.first_child().value() ), targetNode, File::HEADER, conditions, errors ) );
		}
		else if( tagName == "headerpattern" ) {
			processFilePattern( mParentPath, targetNode, File::HEADER, conditions, errors );
		}
		else if( tagName == "resource" ) {
			mFiles.push_back( File( mParentPath, QString::fromUtf8( targetNode.first_child().value() ), targetNode, File::RESOURCE, conditions, errors ) );
		}
		else if( tagName == "asset" ) {
			mFiles.push_back( File( mParentPath, QString::fromUtf8( targetNode.first_child().value() ), targetNode, File::ASSET, conditions, errors ) );
		}
		else if( tagName == "framework" ) {
			mFiles.push_back( File( mParentPath, QString::fromUtf8( targetNode.first_child().value() ), targetNode, File::FRAMEWORK, conditions, errors ) );
		}
		else if( tagName == "buildcopy" ) {
			mFiles.push_back( File( mParentPath, QString::fromUtf8( targetNode.first_child().value() ), targetNode, File::BUILD_COPY, conditions, errors ) );
		}
		/*
		else if( tagName == "directory" ) {
			mFiles.push_back( File( mParentPath, targetNode, File::DIRECTORY, conditions ) );
		}*/
		else if( tagName == "file" ) {
			mFiles.push_back( File( mParentPath, QString::fromUtf8( targetNode.first_child().value() ), targetNode, File::FILE, conditions, errors ) );
		}
		else if( tagName == "staticlibrary" ) {
			mStaticLibraries.push_back( StaticLibrary( mParentPath, targetNode, conditions, errors ) );
		}
		else if( tagName == "dynamiclibrary" ) {
			mDynamicLibraries.push_back( DynamicLibrary( mParentPath, targetNode, conditions, errors ) );
		}
		else if( tagName == "includepath" ) {
			mIncludePaths.push_back( IncludePath( mParentPath, targetNode, conditions, errors ) );
		}
		else if( tagName == "librarypath" ) {
			mLibraryPaths.push_back( LibraryPath( mParentPath, targetNode, conditions, errors ) );
		}
		else if( tagName == "frameworkpath" ) {
			mFrameworkPaths.push_back( FrameworkPath( mParentPath, targetNode, conditions, errors ) );
		}
		else if( tagName == "setting" ) {
			mBuildSettings.push_back( BuildSetting( mParentPath, targetNode, conditions, errors ) );
		}
		else if( tagName == "preprocessordefine" ) {
			mPreprocessorDefines.push_back( PreprocessorDefine( mParentPath, targetNode, conditions, errors ) );
		}
		else if( tagName == "outputextension" ) {
			mOutputExtensions.push_back( OutputExtension( mParentPath, targetNode, conditions, errors ) );
		}
		else if( tagName == "requires" ) {
			mRequires.push_back( QString::fromUtf8( targetNode.first_child().value() ) );
		}
		else if( tagName == "platform" ) {
			parseGroup( targetNode, conditions, errors );
		}
		else if( tagName == "supports" ) {
			parseSupports( targetNode, errors );
		}
		else {
			errors->addWarning( QString("Ignoring unknown tag: <") + tagName + ">" );
		}

		targetNode = targetNode.next_sibling();
	}
}

void Template::parseSupports( const pugi::xml_node &node, ErrorList *errors )
{
	QMap<QString,QString> conditions;
	if( ! node.attribute( "os" ).empty() ) {
		QString os = QString::fromUtf8( node.attribute("os").value() ).toLower();
		conditions["os"] = os;
	}
	if( ! node.attribute( "compiler" ).empty() ) {
		QString compiler = QString::fromUtf8( node.attribute("compiler").value() ).toLower();
		conditions["compiler"] = compiler;
	}
	
	testKnownAttributes( node, QStringList() << "os" << "compiler", "supports", errors );

	mSupports.push_back( conditions );
}

void Template::setOutputPathToInput()
{
	setOutputPath( "", "", "" );
}

void Template::setOutputPath( const QString &outputPath, const QString &replaceName, const QString &cinderPath )
{
	mOutputPath = outputPath;
	mReplacementPrefix = replaceName;
	mCinderPath = cinderPath;
	
	for( QList<File>::Iterator it = mFiles.begin(); it != mFiles.end(); ++it )
		it->setOutputPath( outputPath, replaceName, cinderPath );

	for( QList<IncludePath>::Iterator it = mIncludePaths.begin(); it != mIncludePaths.end(); ++it )
		it->setOutputPath( outputPath, replaceName, cinderPath );

	for( QList<LibraryPath>::Iterator it = mLibraryPaths.begin(); it != mLibraryPaths.end(); ++it )
		it->setOutputPath( outputPath, replaceName, cinderPath );

	for( QList<FrameworkPath>::Iterator it = mFrameworkPaths.begin(); it != mFrameworkPaths.end(); ++it )
		it->setOutputPath( outputPath, replaceName, cinderPath );

	for( QList<StaticLibrary>::Iterator it = mStaticLibraries.begin(); it != mStaticLibraries.end(); ++it )
		it->setOutputPath( outputPath, replaceName, cinderPath );

	for( QList<DynamicLibrary>::Iterator it = mDynamicLibraries.begin(); it != mDynamicLibraries.end(); ++it )
		it->setOutputPath( outputPath, replaceName, cinderPath );
}

void Template::setupVirtualPaths( const QString &virtualPath )
{
	// generate all the files
    for( QList<File>::Iterator fileIt = mFiles.begin(); fileIt != mFiles.end(); ++fileIt ) {
		if( ! fileIt->isOutputSdkRelative() ) {
			fileIt->setVirtualPath( joinPathMac( virtualPath, fileIt->getRelativeInputPath() ) );
			std::cout << "Virtual path now " << qPrintable(fileIt->getVirtualPath()) << std::endl;
		}
		else {
			std::cout << "Ignoring virtual path on " << qPrintable(fileIt->getRelativeInputPath()) << std::endl;
		}
	}
}

bool Template::supportsConditions( const QMap<QString,QString> &conditions ) const
{
	// empty 'mSupports' implies we support everything
	if( mSupports.empty() )
		return true;
	
	for( QList<QMap<QString,QString> >::ConstIterator suppIt = mSupports.begin(); suppIt != mSupports.end(); ++suppIt ) {
		bool matched = true;
		for( QMap<QString,QString>::ConstIterator condIt = conditions.begin(); condIt != conditions.end(); ++condIt ) {
			if( suppIt->contains( condIt.key() ) && (*suppIt)[condIt.key()] != condIt.value() )
				matched = false;
		}
		if( matched )
			return true;
	}

	return false;
}

void Template::instantiateFilesMatchingConditions( const QList<QMap<QString,QString> > &conditionsList, bool overwriteExisting ) const
{
	// files
	for( QList<File>::ConstIterator fileIt = mFiles.begin(); fileIt != mFiles.end(); ++fileIt ) {
		for( QList<QMap<QString,QString> >::ConstIterator conditionsIt = conditionsList.begin(); conditionsIt != conditionsList.end(); ++conditionsIt ) {
			if( fileIt->shouldCopy() && fileIt->conditionsMatch( *conditionsIt ) ) {
				copyFileOrDir( fileIt->getAbsoluteInputPath(), fileIt->getAbsoluteOutputPath(), overwriteExisting, fileIt->getReplaceContents(), mReplacementPrefix, false );
				break;
			}
		}
	}

	// include paths
	for( QList<IncludePath>::ConstIterator pathIt = mIncludePaths.begin(); pathIt != mIncludePaths.end(); ++pathIt ) {
		for( QList<QMap<QString,QString> >::ConstIterator conditionsIt = conditionsList.begin(); conditionsIt != conditionsList.end(); ++conditionsIt ) {
			if( pathIt->shouldCopy() && pathIt->conditionsMatch( *conditionsIt ) ) {
				copyFileOrDir( pathIt->getAbsoluteInputPath(), pathIt->getAbsoluteOutputPath(), overwriteExisting );
				break;
			}
		}
	}

	// dynamic libraries
	for( QList<DynamicLibrary>::ConstIterator libIt = mDynamicLibraries.begin(); libIt != mDynamicLibraries.end(); ++libIt ) {
		for( QList<QMap<QString,QString> >::ConstIterator conditionsIt = conditionsList.begin(); conditionsIt != conditionsList.end(); ++conditionsIt ) {
			if( libIt->shouldCopy() && libIt->conditionsMatch( *conditionsIt ) ) {
				copyFileOrDir( libIt->getAbsoluteInputPath(), libIt->getAbsoluteOutputPath(), overwriteExisting );
				break;
			}
		}
	}

	// static libraries
	for( QList<StaticLibrary>::ConstIterator libIt = mStaticLibraries.begin(); libIt != mStaticLibraries.end(); ++libIt ) {
		for( QList<QMap<QString,QString> >::ConstIterator conditionsIt = conditionsList.begin(); conditionsIt != conditionsList.end(); ++conditionsIt ) {
			if( libIt->shouldCopy() && libIt->conditionsMatch( *conditionsIt ) ) {
				copyFileOrDir( libIt->getAbsoluteInputPath(), libIt->getAbsoluteOutputPath(), overwriteExisting );
				break;
			}
		}
	}
}

QList<Template::File> Template::getFilesMatchingConditions( const QMap<QString,QString> &conditions ) const
{
	QList<File> result;

	for( QList<File>::ConstIterator fileIt = mFiles.begin(); fileIt != mFiles.end(); ++fileIt ) {
		if( fileIt->conditionsMatch( conditions ) ) {
			result.push_back( *fileIt );
		}
	}

	return result;
}

QList<Template::File> Template::getFilesMatchingConditions( const QList<QMap<QString,QString> > &conditionsList ) const
{
    QList<File> result;

    for( QList<File>::ConstIterator fileIt = mFiles.begin(); fileIt != mFiles.end(); ++fileIt ) {
        for( QList<QMap<QString,QString> >::ConstIterator conditionsIt = conditionsList.begin(); conditionsIt != conditionsList.end(); ++conditionsIt ) {
            if( fileIt->conditionsMatch( *conditionsIt ) ) {
                result.push_back( *fileIt );
                break;
            }
        }
    }

    return result;
}

template<typename T>
QList<T> Template::getItemsMatchingConditions( const QList<T> &list, const QMap<QString,QString> &conditions ) const
{
	QList<T> result;

	for( typename QList<T>::ConstIterator fileIt = list.begin(); fileIt != list.end(); ++fileIt ) {
		if( fileIt->conditionsMatch( conditions ) ) {
			result.push_back( *fileIt );
		}
	}

	return result;  
}

QList<Template::IncludePath> Template::getIncludePathsMatchingConditions( const QMap<QString,QString> &conditions ) const
{
	return getItemsMatchingConditions<IncludePath>( mIncludePaths, conditions );
}

QList<Template::LibraryPath> Template::getLibraryPathsMatchingConditions( const QMap<QString,QString> &conditions ) const
{
	return getItemsMatchingConditions<LibraryPath>( mLibraryPaths, conditions );
}

QList<Template::FrameworkPath> Template::getFrameworkPathsMatchingConditions( const QMap<QString,QString> &conditions ) const
{
	return getItemsMatchingConditions<FrameworkPath>( mFrameworkPaths, conditions );
}

QList<Template::StaticLibrary> Template::getStaticLibrariesMatchingConditions( const QMap<QString,QString> &conditions ) const
{
	return getItemsMatchingConditions<StaticLibrary>( mStaticLibraries, conditions );
}

QList<Template::DynamicLibrary> Template::getDynamicLibrariesMatchingConditions( const QMap<QString,QString> &conditions ) const
{
	return getItemsMatchingConditions<DynamicLibrary>( mDynamicLibraries, conditions );
}

QList<Template::BuildSetting> Template::getBuildSettingsMatchingConditions( const QMap<QString,QString> &conditions ) const
{
	return getItemsMatchingConditions<BuildSetting>( mBuildSettings, conditions );
}

QList<Template::PreprocessorDefine> Template::getPreprocessorDefinesMatchingConditions( const QMap<QString,QString> &conditions ) const
{
	return getItemsMatchingConditions<PreprocessorDefine>( mPreprocessorDefines, conditions );
}

QList<Template::OutputExtension> Template::getOutputExtensionsMatchingConditions( const QMap<QString,QString> &conditions ) const
{
	return getItemsMatchingConditions<OutputExtension>( mOutputExtensions, conditions );
}
