#pragma once

#include "TinderBox.h"
#include "Template.h"

class CinderBlock;
typedef QSharedPointer<CinderBlock>		CinderBlockRef;

class CinderBlock : public Template
{
  public:
	CinderBlock( const QString &dir, const pugi::xml_node &node, ErrorList *errors );

	typedef enum { INSTALL_NONE, INSTALL_COPY, INSTALL_REFERENCE, INSTALL_GIT_SUBMODULE } InstallType;

	const QString&		getAuthor() const { return mAuthor; }
	const QString&		getGitUrl() const { return mGitUrl; }
	const QString&		getBlockUrl() const { return mBlockUrl; }
	const QString&		getDescription() const { return mDescription; }
	const QString&		getLibraryUrl() const { return mLibraryUrl; }
	const QString&		getLicense() const { return mLicense; }
	const QString&		getVersion() const { return mVersion; }
	
	const QString&		getIconPath() const { return mIconPath; }
	void				setIconPath( const QString &iconPath ) { mIconPath = iconPath; }
	
	InstallType			getInstallType() const { return mInstallType; }
	void				setInstallType( const InstallType installType ) { mInstallType = installType; }

	bool				isRequired() const { return mRequired; }
	void				setRequired( bool required ) { mRequired = required; }

  protected:
	QString			mAuthor;
	QString			mGitUrl, mBlockUrl;
	QString			mDescription;
	QString			mLibraryUrl;
	QString			mLicense;
	QString			mIconPath;
	QString			mVersion;
	bool			mRequired;
	InstallType		mInstallType;
};
