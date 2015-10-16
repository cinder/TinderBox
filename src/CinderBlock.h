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
