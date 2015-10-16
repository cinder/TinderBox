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

#include "XCodeProj.h"

#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <QUuid>
#include <qfileinfo.h>

XCodeProj::XCodeProj( QSharedPointer<PList> plist )
	: mPList( plist )
{
	QSharedPointer<PList::Dictionary> contentsDict = plist->getContents().dynamicCast<PList::Dictionary>();
	if( contentsDict ) {
		QSharedPointer<PList::Dictionary> objectsDict = contentsDict->get<PList::Dictionary>( "objects" );
		if( ! objectsDict )
			throw XCodeProjExc( "Unable to find objects dictionary" );
		for( PList::Dictionary::Iterator itIt = objectsDict->begin(); itIt != objectsDict->end(); ++itIt ) {
			QSharedPointer<PList::Dictionary> itDict = itIt->dynamicCast<PList::Dictionary>();
			if( itDict ) {
				QSharedPointer<PList::ValueString> isa = itDict->get<PList::ValueString>( "isa" );
				if( ! isa )
					continue;
				if( isa->getValue() == "PBXBuildFile" ) {
					mPbxBuildFiles.push_back( QSharedPointer<PbxBuildFile>( new PbxBuildFile( itDict ) ) );
				}
				else if( isa->getValue() == "PBXFileReference" ) {
					mPbxFileReferences.push_back( QSharedPointer<PbxFileReference>( new PbxFileReference( itDict ) ) );
				}
				else if( isa->getValue() == "PBXFrameworksBuildPhase" || isa->getValue() == "PBXHeadersBuildPhase" ||
						isa->getValue() == "PBXSourcesBuildPhase" || isa->getValue() == "PBXResourcesBuildPhase" || isa->getValue() == "PBXShellScriptBuildPhase" ) {
					mPbxBuildPhases.push_back( QSharedPointer<PbxBuildPhase>( new PbxBuildPhase( itDict ) ) );
				}
				else if( isa->getValue() == "PBXGroup" ) {
					mPbxGroups.push_back( QSharedPointer<PbxGroup>( new PbxGroup( itDict ) ) );
				}
				else if( isa->getValue() == "PBXNativeTarget" ) {
					mPbxNativeTargets.push_back( QSharedPointer<PbxNativeTarget>( new PbxNativeTarget( itDict ) ) );
				}
				else if( isa->getValue() == "XCBuildConfiguration" ) {
					mXcBuildConfigurations.push_back( QSharedPointer<XcBuildConfiguration>( new XcBuildConfiguration( itDict ) ) );
				}
				else if( isa->getValue() == "XCConfigurationList" ) {
					mXcConfigurationLists.push_back( QSharedPointer<XcConfigurationList>( new XcConfigurationList( itDict ) ) );
				}
				else if( isa->getValue() == "PBXProject" ) {
					mPbxProjects.push_back( QSharedPointer<PbxProject>( new PbxProject( itDict ) ) );
				}
				else if( isa->getValue() == "PBXBuildRule" ) {
					mPbxBuildRules.push_back( QSharedPointer<PbxBuildRule>( new PbxBuildRule( itDict ) ) );
				}
				else if( isa->getValue() == "PBXVariantGroup" ) { // TODO: Unhandled
					mUnhandledItems.push_back( QSharedPointer<PbxItem>( new PbxItem( itDict ) ) );
				}
				else if( isa->getValue() == "XCVersionGroup" ) { // TODO: Unhandled
					mUnhandledItems.push_back( QSharedPointer<PbxItem>( new PbxItem( itDict ) ) );
				}
				else {
					std::cout << "problem: " << qPrintable( isa->getValue() ) << std::endl;
					throw XCodeProjExc( "Contents contains an unknown item type: " + isa->getValue() );
				}
			}
			else
				throw XCodeProjExc( "Contents contains non-dictionary" );
		}
	}
	else {
		throw XCodeProjExc( "Unable to find root dictionary" );
	}

	// give all buildFiles their pointers to their PbxFileRefs
	for( QList<QSharedPointer<PbxBuildFile> >::iterator fileIt = mPbxBuildFiles.begin(); fileIt != mPbxBuildFiles.end(); ++fileIt ) {
		(*fileIt)->setupFileRef( this );
	}

	// setup PbxBuildPhases
	for( QList<QSharedPointer<PbxBuildPhase> >::iterator buildPhaseIt = mPbxBuildPhases.begin(); buildPhaseIt != mPbxBuildPhases.end(); ++buildPhaseIt ) {
		(*buildPhaseIt)->setupBuildFiles( this );
	}

	// setup pbxGroups
	for( QList<QSharedPointer<PbxGroup> >::iterator groupIt = mPbxGroups.begin(); groupIt != mPbxGroups.end(); ++groupIt ) {
		(*groupIt)->setupFileReferences( this );
	}

	// setup PbxNativeTargets
	for( QList<QSharedPointer<PbxNativeTarget> >::iterator it = mPbxNativeTargets.begin(); it != mPbxNativeTargets.end(); ++it) {
		(*it)->setupReferences( this );
	}

	// setup XcConfigurationLists
	for( QList<QSharedPointer<XcConfigurationList> >::iterator it = mXcConfigurationLists.begin(); it != mXcConfigurationLists.end(); ++it) {
		(*it)->setupReferences( this );
	}

	// setup PbxProjects
	for( QList<QSharedPointer<PbxProject> >::iterator it = mPbxProjects.begin(); it != mPbxProjects.end(); ++it) {
		(*it)->setupReferences( this );
	}

	// setup XCodeProj's mainGroup
	PList::ValueStringRef rootObjVal = contentsDict->get<PList::ValueString>( "rootObject" );
	if( ! rootObjVal )
		throw XCodeProjExc( "No rootObject" );
	QString rootObjTag = rootObjVal->getValue();
	QSharedPointer<PbxProject> proj = findItem<PbxProject>( rootObjTag );
	if( ! proj )
		throw XCodeProjExc( "Unable to locate referenced rootObject" );
	mMainGroup = proj->getMainGroup();
	if( ! mMainGroup )
		throw XCodeProjExc( "Unable to project's mainGroup" );


	setupLineComments();
}

XCodeProjRef XCodeProj::createFromFilePath( QString path )
{
	QFile qf( path );
	QSharedPointer<PList> plist( PList::create( qf ) );
	return QSharedPointer<XCodeProj>( new XCodeProj( plist ) );
}

XCodeProjRef XCodeProj::createFromString( const QString &s )
{
	QSharedPointer<PList> plist( PList::create( s ) );
	return QSharedPointer<XCodeProj>( new XCodeProj( plist ) );
}

QSharedPointer<XCodeProj::PbxItem>	XCodeProj::findItemGeneric( const QString &tag )
{
	for( QList<QSharedPointer<PbxBuildFile> >::iterator itIt = mPbxBuildFiles.begin(); itIt != mPbxBuildFiles.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	for( QList<QSharedPointer<PbxFileReference> >::iterator itIt = mPbxFileReferences.begin(); itIt != mPbxFileReferences.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	for( QList<QSharedPointer<PbxBuildPhase> >::iterator itIt = mPbxBuildPhases.begin(); itIt != mPbxBuildPhases.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	for( QList<QSharedPointer<PbxBuildRule> >::iterator itIt = mPbxBuildRules.begin(); itIt != mPbxBuildRules.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	for( QList<QSharedPointer<PbxGroup> >::iterator itIt = mPbxGroups.begin(); itIt != mPbxGroups.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	for( QList<QSharedPointer<PbxNativeTarget> >::iterator itIt = mPbxNativeTargets.begin(); itIt != mPbxNativeTargets.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	for( QList<QSharedPointer<XcBuildConfiguration> >::iterator itIt = mXcBuildConfigurations.begin(); itIt != mXcBuildConfigurations.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	for( QList<QSharedPointer<XcConfigurationList> >::iterator itIt = mXcConfigurationLists.begin(); itIt != mXcConfigurationLists.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	for( QList<QSharedPointer<PbxProject> >::iterator itIt = mPbxProjects.begin(); itIt != mPbxProjects.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	for( QList<QSharedPointer<PbxItem> >::iterator itIt = mUnhandledItems.begin(); itIt != mUnhandledItems.end(); ++itIt )
		if( (*itIt)->getTag() == tag )
			return *itIt;

	return QSharedPointer<XCodeProj::PbxItem>();
}

void XCodeProj::print( std::ostream &os )
{
	mPList->print( os );
}

void XCodeProj::setupLineComments()
{
	QString lastIsa = "";
	QList<QPair<PList::ItemRef,PList::ItemRef> > insertions;

	QSharedPointer<PList::Dictionary> contentsDict = mPList->getContents().dynamicCast<PList::Dictionary>();
	if( contentsDict ) {
		QSharedPointer<PList::Dictionary> objectsDict = contentsDict->get<PList::Dictionary>( "objects" );
		if( ! objectsDict )
			throw XCodeProjExc( "Unable to find objects dictionary" );
		for( PList::Dictionary::Iterator itIt = objectsDict->begin(); itIt != objectsDict->end(); ++itIt ) {
			QSharedPointer<PList::Dictionary> itDict = itIt->dynamicCast<PList::Dictionary>();
			if( itDict ) {
				QSharedPointer<PList::ValueString> isaVal = itDict->get<PList::ValueString>( "isa" );
				if( ! isaVal )
					continue;

				else if( isaVal->getValue() != lastIsa ) {
					if( ! lastIsa.isEmpty() )
						insertions.push_back( QPair<PList::ItemRef,PList::ItemRef>( *itIt, PList::LineComment::create( QString( "/* End ") + lastIsa + QString(" section */" ) ) ) );
					insertions.push_back( QPair<PList::ItemRef,PList::ItemRef>( *itIt, PList::LineComment::create( QString( "\n/* Begin ") + isaVal->getValue() + QString(" section */" ) ) ) );
					lastIsa = isaVal->getValue();
				}
			}
		}
		// now make the insertions
		for( QList<QPair<PList::ItemRef,PList::ItemRef> >::Iterator insertIt = insertions.begin(); insertIt != insertions.end(); ++insertIt )
			mPList->insertBefore( insertIt->first, insertIt->second );

		// don't forget the end of the last guy
		mPList->insertAfter( objectsDict->getItems().back(), PList::LineComment::create( QString( "/* End ") + lastIsa + QString(" section */" ) ) );
	}
}

QSharedPointer<XCodeProj::PbxBuildPhase> XCodeProj::findBuildPhase( const QString &isa )
{
	for( QList<QSharedPointer<PbxBuildPhase> >::Iterator itIt = mPbxBuildPhases.begin(); itIt != mPbxBuildPhases.end(); ++itIt )
		if( (*itIt)->getIsa() == isa )
			return *itIt;

	return QSharedPointer<PbxBuildPhase>();
}

QSharedPointer<XCodeProj::PbxBuildPhase> XCodeProj::createOrFindCopyFilesBuildPhase( int dstSubfolderSpec )
{
	// try to find an existing copyFiles buildPhase with the same dstSubfolderSpec
	for( QList<QSharedPointer<PbxBuildPhase> >::Iterator itIt = mPbxBuildPhases.begin(); itIt != mPbxBuildPhases.end(); ++itIt ) {
		if( (*itIt)->getIsa() == QString( "PBXCopyFilesBuildPhase" ) ) {
			if( (**itIt)["dstSubfolderSpec"] == QString( dstSubfolderSpec ) )
				return *itIt;
		}
	}
	
	// no dice - let's make one
	QString fileRefTag = generateUniqueTag();
	QSharedPointer<PbxBuildPhase> result( new PbxBuildPhase( fileRefTag, "PBXCopyFilesBuildPhase" ) );
	(*result)["dstPath"] = QString();
	(*result)["dstSubfolderSpec"] = QString::number( dstSubfolderSpec );
	result->getPListItem()->addItem( PList::Array::create( "files" ) );
	(*result)["runOnlyForDeploymentPostprocessing"] = QString::number( 0 );
	result->getPListItem()->setTagSuffixComment( "Copy Files", true );
	
	addBuildPhase( result );
	return result;
}

void XCodeProj::addBuildPhase( QSharedPointer<PbxBuildPhase> buildPhase )
{
	mPbxBuildPhases.push_back( buildPhase );
	QSharedPointer<PList::Array> buildPhasesArray = getTarget()->mPListItem->get<PList::Array>( "buildPhases" );
	PList::ValueStringRef buildPhaseTag = PList::ValueString::create( "", buildPhase->getTag() );
	buildPhaseTag->setValueSuffixComment( buildPhase->getHumanReadableName(), true );
	buildPhasesArray->addItem( buildPhaseTag );
	QSharedPointer<PList::Dictionary> contentsDict = mPList->getContents().dynamicCast<PList::Dictionary>();
	QSharedPointer<PList::Dictionary> objectsDict = contentsDict->get<PList::Dictionary>( "objects" );
	objectsDict->addItem( buildPhase->getPListItem() );
}

QSharedPointer<XCodeProj::XcBuildConfiguration> XCodeProj::findBuildConfiguration( QSharedPointer<XCodeProj::XcConfigurationList> configList, QString name )
{
	QList<QString> configTags = configList->getConfigurationTags();

	for( QList<QString>::Iterator configTagIt = configTags.begin(); configTagIt != configTags.end(); ++configTagIt ) {
		for( QList<QSharedPointer<XcBuildConfiguration> >::Iterator buildConfigIt = mXcBuildConfigurations.begin(); buildConfigIt != mXcBuildConfigurations.end(); ++buildConfigIt ) {
			if( (*buildConfigIt)->getPListItem()->getTag() == *configTagIt ) { // found the config matching this configTagIt; now does it have the right name?
				if( (*buildConfigIt)->getName().compare( name, Qt::CaseInsensitive ) == 0 )
					return *buildConfigIt;
				break;
			}
		}
	}

	return QSharedPointer<XCodeProj::XcBuildConfiguration>();
}

QSharedPointer<XCodeProj::PbxGroup> XCodeProj::findSourcesGroup()
{
	const QString names[] = { "source", "src", "sources" };

	for( QList<QSharedPointer<PbxGroup> >::Iterator groupIt = mPbxGroups.begin(); groupIt != mPbxGroups.end(); ++groupIt ) {
		if( (*groupIt)->getName().compare( names, Qt::CaseInsensitive ) == 0 )
			return *groupIt;
	}

	// couldn't find it - just return the root group
	return getMainGroup();
}

QSharedPointer<XCodeProj::PbxGroup> XCodeProj::findHeadersGroup()
{
	const QString names[] = { "headers", "include", "header" };

	for( QList<QSharedPointer<PbxGroup> >::Iterator groupIt = mPbxGroups.begin(); groupIt != mPbxGroups.end(); ++groupIt ) {
		if( (*groupIt)->getName().compare( names, Qt::CaseInsensitive ) == 0 )
			return *groupIt;
	}

	// couldn't find it - just return the root group
	return getMainGroup();
}

QSharedPointer<XCodeProj::PbxGroup> XCodeProj::findResourcesGroup()
{
	const QString names[] = { "resources", "rsrc" };

	for( QList<QSharedPointer<PbxGroup> >::Iterator groupIt = mPbxGroups.begin(); groupIt != mPbxGroups.end(); ++groupIt ) {
		if( (*groupIt)->getName().compare( names, Qt::CaseInsensitive ) == 0 )
			return *groupIt;
	}

	// couldn't find it - just return the root group
	return getMainGroup();
}

QSharedPointer<XCodeProj::PbxGroup> XCodeProj::findFrameworksGroup()
{
	const QString names[] = { "frameworks", "framework" };

	for( QList<QSharedPointer<PbxGroup> >::Iterator groupIt = mPbxGroups.begin(); groupIt != mPbxGroups.end(); ++groupIt ) {
		if( (*groupIt)->getName().compare( names, Qt::CaseInsensitive ) == 0 )
			return *groupIt;
	}

	// couldn't find it - just return the root group
	return getMainGroup();
}

QSharedPointer<XCodeProj::PbxGroup> XCodeProj::createAndFindGroupsPath( QSharedPointer<XCodeProj::PbxGroup> relativeTo, const QString &virtualPath )
{
    QStringList items = virtualPath.split( '/' );
    QSharedPointer<PbxGroup> result = relativeTo;
    for( QStringList::Iterator item = items.begin(); item != (items.end() - 1); ++item ) {
        QSharedPointer<PbxGroup> existingGroup = result->findChildGroup( *item );
        if( existingGroup )
            result = existingGroup;
        else {
            QSharedPointer<PbxGroup> newGroup = PbxGroup::create( generateUniqueTag(), *item );
            mPList->insertBefore( result->getPListItem(), newGroup->getPListItem() );            
            result->add( newGroup );
            result = newGroup;
        }
    }

    return result;
}

QString XCodeProj::generateUniqueTag() const
{
	QList<QString> tags;

	QSharedPointer<PList::Dictionary> contentsDict = mPList->getContents().dynamicCast<PList::Dictionary>();
	QSharedPointer<PList::Dictionary> objectsDict = contentsDict->get<PList::Dictionary>( "objects" );
	for( PList::Dictionary::Iterator itIt = objectsDict->begin(); itIt != objectsDict->end(); ++itIt ) {
		tags.push_back( (*itIt)->getTag() );
	}

	QString result;
	do {
		QUuid uid = QUuid::createUuid();
		result = uid.toString();
		result = result.remove( '-' );
		result = result.left( 25 ).right( 24 ).toUpper();
	} while( std::find( tags.begin(), tags.end(), result) != tags.end() );

	return result;
}

PList::DictionaryRef XCodeProj::getTargetBuildSettings( const QString &configurationName )
{
	PList::ValueStringRef buildConfigVal = getTarget()->getPListItem()->get<PList::ValueString>( "buildConfigurationList" );
	if( ! buildConfigVal )
		throw XCodeProjExc( "Target has no buildConfigurationList" );
	QSharedPointer<XCodeProj::XcConfigurationList> configList = findItem<XcConfigurationList>( buildConfigVal->getValue() );
	if( ! configList )
		throw XCodeProjExc( "Unable to locate Target configuration list" );
	QSharedPointer<XcBuildConfiguration> debugBuildConfig = findBuildConfiguration( configList, configurationName );
	if( ! debugBuildConfig )
		throw XCodeProjExc( QString("Unable to locate") + configurationName + QString(" Target build configuration") );
	PList::DictionaryRef result = debugBuildConfig->getPListItem()->get<PList::Dictionary>( "buildSettings" );
	if( ! result )
		throw XCodeProjExc( QString("Unable to locate buildSettings of ") + configurationName + QString(" Target build configuration") );
	return result;
}

PList::DictionaryRef XCodeProj::getProjectBuildSettings( const QString &configurationName )
{
	PList::ValueStringRef buildConfigVal = getProject()->getPListItem()->get<PList::ValueString>( "buildConfigurationList" );
	if( ! buildConfigVal )
		throw XCodeProjExc( "Project has no buildConfigurationList" );
	QSharedPointer<XCodeProj::XcConfigurationList> configList = findItem<XcConfigurationList>( buildConfigVal->getValue() );
	if( ! configList )
		throw XCodeProjExc( "Unable to locate Project configuration list" );
	QSharedPointer<XcBuildConfiguration> debugBuildConfig = findBuildConfiguration( configList, configurationName );
	if( ! debugBuildConfig )
		throw XCodeProjExc( QString("Unable to locate") + configurationName + QString(" Project build configuration") );
	PList::DictionaryRef result = debugBuildConfig->getPListItem()->get<PList::Dictionary>( "buildSettings" );
	if( ! result )
		throw XCodeProjExc( QString("Unable to locate buildSettings of ") + configurationName + QString(" Project build configuration") );
	return result;
}

QList<PList::ItemRef> XCodeProj::findDominantBuildSettings( QString configuration, QString sdk, QString settingName, PList::DictionaryRef *resultParentDictionary )
{
	QList<PList::ItemRef> result;
	// the Target settings always override the Project settings, so if it's defined there return that
	PList::DictionaryRef targetSettings = getTargetBuildSettings( configuration );
	QList<PList::ItemRef> targetSettingsItems = targetSettings->getSettings( settingName );
	if( ! targetSettingsItems.empty() ) {
		if( resultParentDictionary )
			*resultParentDictionary = targetSettings;
		
		for( QList<PList::ItemRef>::ConstIterator settingsIt = targetSettingsItems.begin(); settingsIt != targetSettingsItems.end(); ++settingsIt ) {
			if( (*settingsIt)->sdkMatches( sdk ) )
				result.push_back( *settingsIt );
		}
		return result;
	}

	PList::DictionaryRef projectSettings = getProjectBuildSettings( configuration );
	QList<PList::ItemRef> projectSettingsItems = projectSettings->getSettings( settingName );
	if( ! projectSettingsItems.empty() ) {
		if( resultParentDictionary )
			*resultParentDictionary = projectSettings;

		for( QList<PList::ItemRef>::ConstIterator settingsIt = projectSettingsItems.begin(); settingsIt != projectSettingsItems.end(); ++settingsIt ) {
			if( (*settingsIt)->sdkMatches( sdk ) )
				result.push_back( *settingsIt );
		}
	}

	// nowhere
	return result;
}

void appendSettingToSpecificArray( PList::ItemRef existingVal, PList::DictionaryRef parentDict, const QString &settingName, const QString &settingValue )
{
	if( typeid(*existingVal) == typeid(PList::Array) ) { // good; already an array
		PList::ArrayRef existingArray = existingVal.dynamicCast<PList::Array>();
		existingArray->addItem( PList::ValueString::create( "", settingValue ) );
	}
	else if( typeid(*existingVal) == typeid(PList::ValueString) ) { // currently a string; we'll need to make that into an array
		PList::ValueStringRef existingValString = existingVal.dynamicCast<PList::ValueString>();
		PList::ArrayRef newArray = PList::Array::create( settingName );
		newArray->addItem( PList::ValueString::create( "", existingValString->getValue(), existingValString->isQuoted() ) );
		newArray->addItem( PList::ValueString::create( "", settingValue ) );
		parentDict->replaceItem( existingVal, newArray );
	}
	else // what is this?
		throw XCodeProjExc( "Invalid build setting: " + settingName );	
}

void XCodeProj::addPreprocessorDefine( const QString &config, const QString &sdk, const QString &value )
{
	appendBuildSettingArray( config, sdk, "GCC_PREPROCESSOR_DEFINITIONS", value, true );
}

void XCodeProj::setBuildSetting( const QString &configuration, const QString &sdk, const QString &settingName, const QString &settingValue, bool favorTarget )
{
	PList::DictionaryRef parentDict;
	QList<PList::ItemRef> existingVals = findDominantBuildSettings( configuration, sdk, settingName, &parentDict );
	PList::ValueStringRef newValue = PList::ValueString::create( settingName, settingValue );
	if( ! existingVals.empty() ) { // something already there
		for( QList<PList::ItemRef>::ConstIterator valIt = existingVals.begin(); valIt != existingVals.end(); ++valIt )
			parentDict->replaceItem( *valIt, newValue );
	}
	else { // not here; need to create it
		// first, is there a setting we can copy with an empty sdk?
/*		QList<PList::ItemRef> existingVals = findDominantBuildSettings( configuration, "", settingName, &parentDict );
		if( existingVals.size() == 1 ) {
			PList::ItemRef newItem = (*existingVals.begin())->clone();
			appendSettingToSpecificArray( newItem, parentDict, settingName, settingValue );
		}
		else */{ // nothing to copy from; just make it from scratch
			if( favorTarget )
				getTargetBuildSettings( configuration )->addItem( newValue );
			else
				getProjectBuildSettings( configuration )->addItem( newValue );
		}
	}
}

void XCodeProj::appendBuildSettingArray( const QString &configuration, const QString &sdk, const QString &settingName, const QString &settingValue, bool favorTarget )
{
	PList::DictionaryRef parentDict;
	QList<PList::ItemRef> existingVals = findDominantBuildSettings( configuration, sdk, settingName, &parentDict );
	if( ! existingVals.empty() ) { // something already there
		for( QList<PList::ItemRef>::ConstIterator valIt = existingVals.begin(); valIt != existingVals.end(); ++valIt )
			appendSettingToSpecificArray( *valIt, parentDict, settingName, settingValue );
	}
	else { // not here; need to create it
		// first, is there a setting we can copy with an empty sdk?
		QList<PList::ItemRef> existingVals = findDominantBuildSettings( configuration, "", settingName, &parentDict );
		if( existingVals.size() == 1 ) {
			PList::ItemRef newItem = (*existingVals.begin())->clone();
			appendSettingToSpecificArray( newItem, parentDict, settingName, settingValue );
		}
		else { // nothing to copy from; just make it from scratch
			if( favorTarget )
				createBuildSettingArray( getTargetBuildSettings( configuration ), sdk, settingName, settingValue );
			else
				createBuildSettingArray( getProjectBuildSettings( configuration ), sdk, settingName, settingValue );
		}
	}
}

void XCodeProj::createBuildSettingArray( PList::DictionaryRef settings, const QString &sdk, const QString &settingName, const QString &settingValue )
{
	QString fullSettingName = settingName; // name width SDK appended
	if( ! sdk.isEmpty() )
		fullSettingName += QString::fromUtf8( "[sdk=" ) + sdk + "]";
	PList::ArrayRef newArray = PList::Array::create( fullSettingName );
	newArray->addItem( PList::ValueString::create( "", settingValue ) );
	settings->addItem( newArray );
}

QString XCodeProj::getFileType( const QString &fileSystemPath, const QString &extensionOverride )
{
	QString ext = ( extensionOverride.isEmpty() ? QFileInfo( fileSystemPath ).completeSuffix().toLower() : extensionOverride );
	
		 if( ext == "cpp" ) return "sourcecode.c.cpp";
	else if( ext == "mm" ) return "sourcecode.cpp.objcpp";
	else if( ext == "h" ) return "sourcecode.c.h";
	else if( ext == "plist" ) return "text.plist.xml";
	else if( ext == "icns" ) return "image.icns";
	else if( ext == "framework" ) return "wrapper.framework";
	else if( ext == "dylib" ) return "compiled.mach-o.dylib";
	else
		return "\"\"";
}

void XCodeProj::addSourceFile( const QString &fileSystemPath, const QString &virtualPath, const QString &compileAsExtension )
{
	// setup PbxFileReference
	QString fileRefTag = generateUniqueTag();
	QString fileName = fileSystemPath.section( '/', -1 );
	QSharedPointer<PbxFileReference> newFileRef( new PbxFileReference( fileRefTag ) );
	if( compileAsExtension.isEmpty() || ( compileAsExtension == QFileInfo(fileSystemPath).completeSuffix().toLower() ) )
		(*newFileRef)["lastKnownFileType"] = getFileType( fileSystemPath, compileAsExtension );
	else {
		newFileRef->getPListItem()->erase( "lastKnownFileType" );
		(*newFileRef)["explicitFileType"] = getFileType( fileSystemPath, compileAsExtension );
	}
    (*newFileRef)["path"] = fileSystemPath;
    (*newFileRef)["name"] = fileName;
    newFileRef->getPListItem()->setTagSuffixComment( fileName, true );
	mPList->insertAfter( mPbxFileReferences.back()->getPListItem(), newFileRef->getPListItem() );
	mPbxFileReferences.push_back( newFileRef );

	// setup PbxBuildFile
	QString buildFileTag = generateUniqueTag();
	PbxBuildFile buildFile( buildFileTag, newFileRef );
	buildFile.getPListItem()->setTagSuffixComment( buildFile.getReferencedFileName() + " in Sources", true );
	mPList->insertAfter( mPbxBuildFiles.back()->getPListItem(), buildFile.getPListItem() );

	// add to PBXSourcesBuildPhase
	QSharedPointer<PbxBuildPhase> sourcesBuildPhase = findBuildPhase( "PBXSourcesBuildPhase" );
	sourcesBuildPhase->add( buildFile );

	// if no virtual path, add to sources
	if( virtualPath.length() == 0 )
		findSourcesGroup()->add( newFileRef );
	else
        createAndFindGroupsPath( getMainGroup(), virtualPath )->add( newFileRef );
}

void XCodeProj::addFramework( const QString &fileSystemPath, const QString &virtualPath, bool absolute, bool sdkRelative )
{
	QString fileRefTag = generateUniqueTag();
	QString fileName = fileSystemPath.section( '/', -1 );

	// see if we already have a framework of this name
	if( ! findExistingFileReference( "wrapper.framework", fileName ).isNull() ) {
		return;
	}

	// setup PbxFileReference
	QSharedPointer<PbxFileReference> newFileRef( new PbxFileReference( fileRefTag ) );
	(*newFileRef)["lastKnownFileType"] = getFileType( fileSystemPath );
    (*newFileRef)["path"] = fileSystemPath;
    (*newFileRef)["name"] = fileName;
    if( sdkRelative ) (*newFileRef)["sourceTree"] = "SDKROOT";
    else if( absolute ) (*newFileRef)["sourceTree"] = "<absolute>";
	else (*newFileRef)["sourceTree"] = "<group>";
	
    newFileRef->getPListItem()->setTagSuffixComment( fileName, true );
	mPList->insertAfter( mPbxFileReferences.back()->getPListItem(), newFileRef->getPListItem() );
	mPbxFileReferences.push_back( newFileRef );

	// setup PbxBuildFile
	QString buildFileTag = generateUniqueTag();
	PbxBuildFile buildFile( buildFileTag, newFileRef );
	buildFile.getPListItem()->setTagSuffixComment( buildFile.getReferencedFileName() + " in Frameworks", true );
	mPList->insertAfter( mPbxBuildFiles.back()->getPListItem(), buildFile.getPListItem() );

	// add to PBXFrameworksBuildPhase
	QSharedPointer<PbxBuildPhase> sourcesBuildPhase = findBuildPhase( "PBXFrameworksBuildPhase" );
	sourcesBuildPhase->add( buildFile );

	// if no virtual path, add to frameworks
	if( virtualPath.length() == 0 )
		findFrameworksGroup()->add( newFileRef );
	else
        createAndFindGroupsPath( getMainGroup(), virtualPath )->add( newFileRef );
}

void XCodeProj::addBuildCopy( const QString &fileSystemPath, const QString &virtualPath, bool absolute, bool sdkRelative, const QString &destinationBuildPhase )
{
	QString fileRefTag = generateUniqueTag();
	QString fileName = fileSystemPath.section( '/', -1 );

	// setup PbxFileReference
	QSharedPointer<PbxFileReference> newFileRef = findExistingFileReference( fileSystemPath );
	if( ! newFileRef ) { // couldn't find existing; make a new file ref
		newFileRef = QSharedPointer<PbxFileReference>( new PbxFileReference( fileRefTag ) );
		(*newFileRef)["lastKnownFileType"] = getFileType( fileSystemPath );
		(*newFileRef)["path"] = fileSystemPath;
		(*newFileRef)["name"] = fileName;
		if( sdkRelative ) (*newFileRef)["sourceTree"] = "SDKROOT";
		else if( absolute ) (*newFileRef)["sourceTree"] = "<absolute>";
		else (*newFileRef)["sourceTree"] = "<group>";

		newFileRef->getPListItem()->setTagSuffixComment( fileName, true );
		mPList->insertAfter( mPbxFileReferences.back()->getPListItem(), newFileRef->getPListItem() );
		mPbxFileReferences.push_back( newFileRef );

		// if no virtual path, add to frameworks; this doesn't really make sense except the only consumer of buildCopy is generally a dylib
		if( virtualPath.length() == 0 )
			findFrameworksGroup()->add( newFileRef );
		else
			createAndFindGroupsPath( getMainGroup(), virtualPath )->add( newFileRef );
	}

	// setup PbxBuildFile
	QString buildFileTag = generateUniqueTag();
	PbxBuildFile buildFile( buildFileTag, newFileRef );
	buildFile.getPListItem()->setTagSuffixComment( buildFile.getReferencedFileName() + " in CopyFiles", true );
	mPList->insertAfter( mPbxBuildFiles.back()->getPListItem(), buildFile.getPListItem() );

	// add to PBXCopyFilesBuildPhase
	int dstSubfolderSpec = 6; // Applications
	if( destinationBuildPhase.toLower() == QString("frameworks") )
		dstSubfolderSpec = 10;
	else if( destinationBuildPhase.toLower() == QString("plugins") )
		dstSubfolderSpec = 13;
	QSharedPointer<PbxBuildPhase> copyFilesBuildPhase = createOrFindCopyFilesBuildPhase( dstSubfolderSpec );
	copyFilesBuildPhase->add( buildFile );
}

void XCodeProj::addHeaderFile( const QString &fileSystemPath, const QString &virtualPath )
{
	// setup PbxFileReference
	QString fileRefTag = generateUniqueTag();
	QString fileName = fileSystemPath.section( '/', -1 );
	QSharedPointer<PbxFileReference> newFileRef( new PbxFileReference( fileRefTag ) );
	(*newFileRef)["lastKnownFileType"] = getFileType( fileSystemPath );
	(*newFileRef)["path"] = fileSystemPath;
	(*newFileRef)["name"] = fileName;
	newFileRef->getPListItem()->setTagSuffixComment( fileName, true );
	mPList->insertAfter( mPbxFileReferences.back()->getPListItem(), newFileRef->getPListItem() );
	mPbxFileReferences.push_back( newFileRef );

	// setup PbxBuildFile
	QString buildFileTag = generateUniqueTag();
	PbxBuildFile buildFile( buildFileTag, newFileRef );
	buildFile.getPListItem()->setTagSuffixComment( buildFile.getReferencedFileName() + " in Headers", true );
	mPList->insertAfter( mPbxBuildFiles.back()->getPListItem(), buildFile.getPListItem() );

	// if no virtual path, add to resources
	if( virtualPath.length() == 0 )
		findHeadersGroup()->add( newFileRef );
	else
        createAndFindGroupsPath( getMainGroup(), virtualPath )->add( newFileRef );
}

void XCodeProj::addResource( const QString &fileSystemPath, const QString &virtualPath, bool excludeFromBuild )
{
	// setup PbxFileReference
	QString fileRefTag = generateUniqueTag();
	QString fileName = fileSystemPath.section( '/', -1 );
	QSharedPointer<PbxFileReference> newFileRef( new PbxFileReference( fileRefTag ) );
	(*newFileRef)["lastKnownFileType"] = getFileType( fileSystemPath );
	(*newFileRef)["path"] = fileSystemPath;
	(*newFileRef)["name"] = fileName;
	newFileRef->getPListItem()->setTagSuffixComment( fileName, true );
	mPList->insertAfter( mPbxFileReferences.back()->getPListItem(), newFileRef->getPListItem() );
	mPbxFileReferences.push_back( newFileRef );

	// setup PbxBuildFile
	if( ! excludeFromBuild ) {
		QString buildFileTag = generateUniqueTag();
		PbxBuildFile buildFile( buildFileTag, newFileRef );
		buildFile.getPListItem()->setTagSuffixComment( buildFile.getReferencedFileName() + " in Resources", true );
		mPList->insertAfter( mPbxBuildFiles.back()->getPListItem(), buildFile.getPListItem() );
	
		QSharedPointer<PbxBuildPhase> resourcesBuildPhase = findBuildPhase( "PBXResourcesBuildPhase" );
		resourcesBuildPhase->add( buildFile );
	}

	// if no virtual path, add to sources
	if( virtualPath.length() == 0 )
		findResourcesGroup()->add( newFileRef );
	else
        createAndFindGroupsPath( getMainGroup(), virtualPath )->add( newFileRef );
}

void XCodeProj::addStaticLibrary( const QString &config, const QString &sdk, const QString &path )
{
	QString quotedPath = path;
	if( PList::needsQuotes( path ) )
		quotedPath = QString::fromUtf8("\"") + path + "\"";
	appendBuildSettingArray( config, sdk, "OTHER_LDFLAGS", quotedPath, true );
}

void XCodeProj::addDynamicLibrary( const QString &config, const QString &sdk, const QString &path )
{
	QString quotedPath = path;
	if( PList::needsQuotes( path ) )
		quotedPath = QString::fromUtf8("\"") + path + "\"";
	appendBuildSettingArray( config, sdk, "OTHER_LDFLAGS", quotedPath, true );
}

void XCodeProj::addSystemHeaderPath( const QString &config, const QString &sdk, const QString &path )
{
	QString quotedPath = path;
	if( PList::needsQuotes( path ) )
		quotedPath = QString::fromUtf8("\"") + path + "\"";
	appendBuildSettingArray( config, sdk, "HEADER_SEARCH_PATHS", quotedPath, false );
}

void XCodeProj::addUserHeaderPath( const QString &config, const QString &sdk, const QString &path )
{
	QString quotedPath = path;
	if( PList::needsQuotes( path ) )
		quotedPath = QString::fromUtf8("\"") + path + "\"";
	appendBuildSettingArray( config, sdk, "USER_HEADER_SEARCH_PATHS", quotedPath, false );
}

void XCodeProj::addLibraryPath( const QString &config, const QString &sdk, const QString &path )
{
	QString quotedPath = path;
	if( PList::needsQuotes( path ) )
		quotedPath = QString::fromUtf8("\"") + path + "\"";
	appendBuildSettingArray( config, sdk, "LIBRARY_SEARCH_PATHS", quotedPath, true );
}

void XCodeProj::addFrameworkPath( const QString &config, const QString &sdk, const QString &path )
{
	QString quotedPath = path;
	if( PList::needsQuotes( path ) )
		quotedPath = QString::fromUtf8("\"") + path + "\"";
	appendBuildSettingArray( config, sdk, "FRAMEWORK_SEARCH_PATHS", quotedPath, true );
}

QSharedPointer<XCodeProj::PbxFileReference> XCodeProj::findExistingFileReference( const QString &fileSystemPath )
{
	for( QList<QSharedPointer<PbxFileReference> >::const_iterator itIt = mPbxFileReferences.begin(); itIt != mPbxFileReferences.end(); ++itIt ) {
		const PbxFileReference *item = itIt->data(); // this has to be this way due to const operator[] behaving very differently
		QString existingPath;
		if( item->getPListItem()->contains( "path" ) )
			existingPath = (*item)["path"];

		if( existingPath.toLower() == fileSystemPath.toLower() )
			return *itIt;
	}

	return QSharedPointer<PbxFileReference>();
}

QSharedPointer<XCodeProj::PbxFileReference> XCodeProj::findExistingFileReference( const QString &fileType, const QString &fileName )
{
	for( QList<QSharedPointer<PbxFileReference> >::const_iterator itIt = mPbxFileReferences.begin(); itIt != mPbxFileReferences.end(); ++itIt ) {
		const PbxFileReference *item = itIt->data(); // this has to be this way due to const operator[] behaving very differently
		QString existingLastKnown;
		if( item->getPListItem()->contains( "explicitFileType" ) )
			existingLastKnown = (*item)["explicitFileType"];
		else
			existingLastKnown = (*item)["lastKnownFileType"];
		QString existingFileName = (*item)["name"];
		if( ( existingLastKnown.toLower() == fileType.toLower() ) && ( existingFileName.toLower() == fileName.toLower() ) )
			return *itIt;
	}

	return QSharedPointer<PbxFileReference>();
}

///////////////////////////////////////////////////////////////////////
// PbxItem
XCodeProj::PbxItem::PbxItem( QSharedPointer<PList::Dictionary> plistItem )
	: mPListItem( plistItem )
{
}

///////////////////////////////////////////////////////////////////////
// PbxBuildFile
XCodeProj::PbxBuildFile::PbxBuildFile( QSharedPointer<PList::Dictionary> plistItem )
	: PbxItem( plistItem )
{
	mPListItem->setCarriageReturnsOnContents( false );
}

XCodeProj::PbxBuildFile::PbxBuildFile( const QString &tag, QSharedPointer<PbxFileReference> fileRef )
	: PbxItem( PList::DictionaryRef( new PList::Dictionary( tag ) ) ), mFileRef( fileRef )
{
	QSharedPointer<PList::Dictionary> dict = mPListItem.dynamicCast<PList::Dictionary>();
	mPListItem->setCarriageReturnsOnContents( false );
	dict->addItem( PList::ValueString::create( "isa", "PBXBuildFile" ) );
	PList::ValueStringRef fileRefVal = PList::ValueString::create( "fileRef", fileRef->getTag(), false );
	fileRefVal->setValueSuffixComment( fileRef->getPathFileName(), true );
	dict->addItem( fileRefVal );
}

void XCodeProj::PbxBuildFile::setupFileRef( XCodeProj *proj )
{
	QSharedPointer<XCodeProj::PbxFileReference> item( proj->findItem<XCodeProj::PbxFileReference>( getFileRefString() ) );
	if( item ) {
		mFileRef = item;
	}
	else
		throw XCodeProjExc( QString("PBXBuildFile references unknown fileRef: ") + getFileRefString() );
}

QString XCodeProj::PbxBuildFile::getReferencedFileName() const
{
	return mFileRef->getPath().section( '/', -1 );
}

///////////////////////////////////////////////////////////////////////
// PbxFileReference
XCodeProj::PbxFileReference::PbxFileReference( QSharedPointer<PList::Dictionary> plistItem )
	: PbxItem( plistItem )
{
	mPListItem->setCarriageReturnsOnContents( false );
}

XCodeProj::PbxFileReference::PbxFileReference( const QString &tag )
	: PbxItem( PList::DictionaryRef( new PList::Dictionary( tag ) ) )
{
	QSharedPointer<PList::Dictionary> dict = mPListItem.dynamicCast<PList::Dictionary>();
	mPListItem->setCarriageReturnsOnContents( false );
	dict->addItem( PList::ValueString::create( "isa", "PBXFileReference" ) );
	dict->addItem( PList::ValueString::create( "lastKnownFileType", "" ) );
	dict->addItem( PList::ValueString::create( "path", "" ) );
	dict->addItem( PList::ValueString::create( "sourceTree", "<group>" ) );
}

///////////////////////////////////////////////////////////////////////
// PbxBuildPhase
XCodeProj::PbxBuildPhase::PbxBuildPhase( const QString &tag, const QString &isa )
	: PbxItem( PList::DictionaryRef( new PList::Dictionary( tag ) ) )
{
	QSharedPointer<PList::Dictionary> dict = mPListItem.dynamicCast<PList::Dictionary>();
	dict->addItem( PList::ValueString::create( "isa", isa ) );
	dict->addItem( PList::ValueString::create( "buildActionMask", "2147483647" ) );
}

XCodeProj::PbxBuildPhase::PbxBuildPhase( QSharedPointer<PList::Dictionary> plistItem )
	: PbxItem( plistItem )
{
}

void XCodeProj::PbxBuildPhase::setupBuildFiles( XCodeProj *proj )
{
	QSharedPointer<PList::Array> filesArray = mPListItem->get<PList::Array>( "files" );
	for( PList::Array::Iterator fileIt = filesArray->begin(); fileIt != filesArray->end(); ++fileIt ) {
		QSharedPointer<PList::ValueString> fileNameVal = fileIt->dynamicCast<PList::ValueString>();
		if( ! fileNameVal )
			throw XCodeProjExc( "PBX*BuildPhase files contains non-value entity" );
		QSharedPointer<XCodeProj::PbxBuildFile> buildFile = proj->findItem<XCodeProj::PbxBuildFile>( fileNameVal->getValue() );
		if( buildFile ) {
			mBuildFiles.push_back( buildFile );
		}
		else
			throw XCodeProjExc( QString("PBX*BuildPhase references unknown PBXBuildFile: ") + fileNameVal->getValue() );
	}
}

void XCodeProj::PbxBuildPhase::add( PbxBuildFile &buildFile )
{
	QSharedPointer<PList::Array> filesArray = mPListItem->get<PList::Array>( "files" );
	QSharedPointer<PList::ValueString> vs = PList::ValueString::create( "", buildFile.getTag(), false );
	vs->setValueSuffixComment( buildFile.getReferencedFileName() + " in " + getHumanReadableName(), true );
	filesArray->addItem( vs );
}

QString	XCodeProj::PbxBuildPhase::getHumanReadableName() const
{
	if( getIsa() == "PBXHeadersBuildPhase" )
		return QString( "Headers" );
	else if( getIsa() == "PBXFrameworksBuildPhase" )
		return QString( "Frameworks" );
	else if( getIsa() == "PBXSourcesBuildPhase" )
		return QString( "Sources" );
	else if( getIsa() == "PBXResourcesBuildPhase" )
		return QString( "Resources" );
	else if( getIsa() == "PBXShellScriptBuildPhase" )
		return QString( "Shell Scripts" );
	else if( getIsa() == "PBXCopyFilesBuildPhase" )
		return QString( "CopyFiles" );
	else
		return QString();
}

///////////////////////////////////////////////////////////////////////
// PbxGroup
XCodeProj::PbxGroup::PbxGroup( QSharedPointer<PList::Dictionary> plistItem )
	: PbxItem( plistItem )
{
	QSharedPointer<PList::ValueString> nameVal = mPListItem->get<PList::ValueString>( "name" );
	if( nameVal )
		mName = nameVal->getValue();
}

XCodeProj::PbxGroup::PbxGroup( const QString &tag, const QString &name )
    : PbxItem( PList::DictionaryRef( new PList::Dictionary( tag ) ) ), mName( name )
{
    getPListItem()->addItem( PList::ValueString::create( "isa", "PBXGroup" ) );
    getPListItem()->addItem( PList::Array::create( "children" ) );
    getPListItem()->addItem( PList::ValueString::create( "name", mName ) );
    getPListItem()->addItem( PList::ValueString::create( "sourceTree", "<group>" ) );
    getPListItem()->setTagSuffixComment( name, true );
}

void XCodeProj::PbxGroup::setupFileReferences( XCodeProj *proj )
{
	QSharedPointer<PList::Array> filesArray = mPListItem->get<PList::Array>( "children" );
	for( PList::Array::Iterator fileIt = filesArray->begin(); fileIt != filesArray->end(); ++fileIt ) {
		QSharedPointer<PList::ValueString> fileNameVal = fileIt->dynamicCast<PList::ValueString>();
		if( ! fileNameVal )
			throw XCodeProjExc( "PbxGroup files contains non-value entity" );
		QString fileName = fileNameVal->getValue();
		QSharedPointer<XCodeProj::PbxItem> item( proj->findItemGeneric( fileName ) );
		if( item ) {
			if( item.dynamicCast<XCodeProj::PbxFileReference>() || item.dynamicCast<XCodeProj::PbxGroup>() )
				mChildren.push_back( item );
			else
				throw XCodeProjExc( QString("PBXGroup references item of invalid type: ") + fileName );
		}
		else
			throw XCodeProjExc( QString("PBXGroup references unknown item: ") + fileName );	}
}

void XCodeProj::PbxGroup::add( QSharedPointer<PbxFileReference> fileRef )
{
	mChildren.push_back( fileRef );
	QSharedPointer<PList::Array> filesArray = mPListItem->get<PList::Array>( "children" );
	PList::ValueStringRef valString = PList::ValueString::create( "", fileRef->getTag() );
	valString->setValueSuffixComment( fileRef->getPListItem()->getTagSuffixComment() );
	filesArray->addItem( valString );
}

void XCodeProj::PbxGroup::add( QSharedPointer<PbxGroup> group )
{
	mChildren.push_back( group );
	QSharedPointer<PList::Array> filesArray = mPListItem->get<PList::Array>( "children" );
	PList::ValueStringRef valString = PList::ValueString::create( "", group->getTag() );
	valString->setValueSuffixComment( group->getPListItem()->getTagSuffixComment() );
	filesArray->addItem( valString );
}

QSharedPointer<XCodeProj::PbxItem> XCodeProj::PbxGroup::findChildImpl( const QString &tag ) const
{
    for( QList<QSharedPointer<PbxItem> >::ConstIterator childIt = mChildren.begin(); childIt != mChildren.end(); ++childIt )
        if( (*childIt)->getTag() == tag )
            return *childIt;

    return QSharedPointer<PbxItem>();
}

QSharedPointer<XCodeProj::PbxGroup> XCodeProj::PbxGroup::findChildGroup( const QString &name ) const
{
    for( QList<QSharedPointer<PbxItem> >::ConstIterator childIt = mChildren.begin(); childIt != mChildren.end(); ++childIt ) {
		if( typeid(PbxGroup) == typeid(**childIt) ) {
			PbxGroup &group = static_cast<PbxGroup&>( **childIt );
			if( group.getName() == name )
				return childIt->dynamicCast<PbxGroup>();
		}
	}

    return QSharedPointer<PbxGroup>();
}

void XCodeProj::PbxGroup::print( std::ostream &os )
{
    os << std::endl;
}

///////////////////////////////////////////////////////////////////////
// PbxNativeTarget
XCodeProj::PbxNativeTarget::PbxNativeTarget( QSharedPointer<PList::Dictionary> plistItem )
	: PbxItem( plistItem )
{
}

void XCodeProj::PbxNativeTarget::setupReferences( XCodeProj *proj )
{
	// setup build phases
	QSharedPointer<PList::Array> buildPhasesArray = mPListItem->get<PList::Array>( "buildPhases" );
	for( PList::Array::Iterator buildPhaseIt = buildPhasesArray->begin(); buildPhaseIt != buildPhasesArray->end(); ++buildPhaseIt ) {
		QSharedPointer<PList::ValueString> buildPhaseTag = buildPhaseIt->dynamicCast<PList::ValueString>();
		if( ! buildPhaseTag )
			throw XCodeProjExc( "PBXNativeTarget buildPhase contains non-value entity" );
		QSharedPointer<XCodeProj::PbxBuildPhase> buildPhase = proj->findItem<XCodeProj::PbxBuildPhase>( buildPhaseTag->getValue() );
		if( buildPhase ) {
			mBuildPhases.push_back( buildPhase );
		}
		else
			throw XCodeProjExc( QString("PBXNativeTarget buildPhase references unknown PBXBuildFile: ") + buildPhaseTag->getValue() );
	}

	// product reference
	QSharedPointer<PList::ValueString> productReferenceVal = mPListItem->get<PList::ValueString>( "productReference" );
	if( ! productReferenceVal )
		throw XCodeProjExc( "PBXNativeTarget productReference missing" );
	QSharedPointer<XCodeProj::PbxFileReference> productReference( proj->findItem<XCodeProj::PbxFileReference>( productReferenceVal->getValue() ) );
	if( productReference )
		mProductReference = productReference;
	else
		throw XCodeProjExc( "PBXNativeTarget productReference references unknown item:" + productReferenceVal->getValue() );
}

void XCodeProj::PbxNativeTarget::print( std::ostream &os )
{
    os << std::endl;
}

///////////////////////////////////////////////////////////////////////
// PbxBuildRule
XCodeProj::PbxBuildRule::PbxBuildRule( QSharedPointer<PList::Dictionary> plistItem )
	: PbxItem( plistItem )
{
}

///////////////////////////////////////////////////////////////////////
// XcBuildConfiguration
XCodeProj::XcBuildConfiguration::XcBuildConfiguration( QSharedPointer<PList::Dictionary> plistItem )
	: PbxItem( plistItem )
{
	mName = mPListItem->get<PList::ValueString>( "name" )->getValue();
}

///////////////////////////////////////////////////////////////////////
// XcConfigurationList
XCodeProj::XcConfigurationList::XcConfigurationList( QSharedPointer<PList::Dictionary> plistItem )
	: PbxItem( plistItem )
{
}

void XCodeProj::XcConfigurationList::setupReferences( XCodeProj *proj )
{
	// setup buildConfigurations
	QSharedPointer<PList::Array> buildConfigurations = mPListItem->get<PList::Array>( "buildConfigurations" );
	for( PList::Array::Iterator buildConfigIt = buildConfigurations->begin(); buildConfigIt != buildConfigurations->end(); ++buildConfigIt ) {
		QSharedPointer<PList::ValueString> buildConfigTag = buildConfigIt->dynamicCast<PList::ValueString>();
		if( ! buildConfigTag )
			throw XCodeProjExc( "XCConfigurationList buildPhase contains non-value entity" );
		QSharedPointer<XCodeProj::XcBuildConfiguration> buildConfig = proj->findItem<XCodeProj::XcBuildConfiguration>( buildConfigTag->getValue() );
		if( buildConfig ) {
			mBuildConfigurations.push_back( buildConfig );
		}
		else
			throw XCodeProjExc( QString("XCConfigurationList buildPhase references unknown PBXBuildFile: ") + buildConfigTag->getValue() );
	}
}

QList<QString> XCodeProj::XcConfigurationList::getConfigurationTags()
{
	QList<QString> result;

	PList::ArrayRef array = getPListItem()->get<PList::Array>( "buildConfigurations" );
	if( ! array )
		return result;
	for( PList::Array::Iterator arrayIt = array->begin(); arrayIt != array->end(); ++arrayIt ) {
		PList::ValueStringRef item = arrayIt->dynamicCast<PList::ValueString>();
		if( item )
			result.push_back( item->getValue() );
	}

	return result;
}

///////////////////////////////////////////////////////////////////////
// PbxProject
XCodeProj::PbxProject::PbxProject( QSharedPointer<PList::Dictionary> plistItem )
	: PbxItem( plistItem )
{
}

void XCodeProj::PbxProject::setupReferences( XCodeProj *proj )
{
	// setup targets
	QSharedPointer<PList::Array> targets = mPListItem->get<PList::Array>( "targets" );
	for( PList::Array::Iterator targetIt = targets->begin(); targetIt != targets->end(); ++targetIt ) {
		QSharedPointer<PList::ValueString> targetTag = targetIt->dynamicCast<PList::ValueString>();
		if( ! targetTag )
			throw XCodeProjExc( "PBXProject targets contains non-value entity" );
		QSharedPointer<XCodeProj::PbxNativeTarget> target = proj->findItem<XCodeProj::PbxNativeTarget>( targetTag->getValue() );
		if( target ) {
			mTargets.push_back( target );
		}
		else
			throw XCodeProjExc( QString("PBXProject targets references unknown PBXNativeTarget: ") + targetTag->getValue() );
	}

	// setup main group
	QSharedPointer<PList::ValueString> mainGroupVal = mPListItem->get<PList::ValueString>( "mainGroup" );
	if( ! mainGroupVal )
		throw XCodeProjExc( "PBXProject mainGroup missing" );
	QSharedPointer<XCodeProj::PbxGroup> mainGroup( proj->findItem<XCodeProj::PbxGroup>( mainGroupVal->getValue() ) );
	if( mainGroup )
		mMainGroup = mainGroup;
	else
		throw XCodeProjExc( "PBXProject mainGroup references unknown item:" + mainGroupVal->getValue() );
}

void XCodeProj::PbxProject::print( std::ostream &os )
{
    os << std::endl;
}
