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
#include "CinderBlock.h"
#include "ProjectTemplate.h"
#include "ErrorList.h"

#include <QIcon>

class CinderBlockManager
{
  public:
	static const QList<CinderBlock>&		getCinderBlocks() { return inst()->mCinderBlocks; }
	static const QList<ProjectTemplate>&	getProjectTemplates() { return inst()->mProjectTemplates; }
	
	//! add a directory to be scanned for CinderBlocks
	static void		scan( const QString &path, ErrorList *errors );
	static void		clear() { inst()->clearInst(); }
	CinderBlock*	findById( const QString &id );

	static const QIcon&	getIcon( const QString &path ) { return inst()->getIconInst( path ); }

  private:
	static CinderBlockManager*	inst();
	CinderBlockManager();

	void	clearInst();
	void	scanAndParseCinderBlocks( const QDir &cinderDir, const QDir &path, int depth, ErrorList *errorList );
	const QIcon&	getIconInst( const QString &path );

	QList<CinderBlock>		mCinderBlocks;
	QList<ProjectTemplate>	mProjectTemplates;
	QMap<QString,QIcon>		mIconCache;
};
