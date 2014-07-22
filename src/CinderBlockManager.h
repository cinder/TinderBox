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
