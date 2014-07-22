#pragma once

#include <QDir>
#include <QList>
#include <QSharedPointer>

#include "GeneratorBase.h"
#include "CinderBlock.h"
#include "Template.h"

class Instancer {
  public:
    Instancer( const ProjectTemplate &projectTmpl );

	QList<Template::File>			 getFilesMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<Template::File>			 getResourcesMatchingConditions( const QList<QMap<QString,QString> > &conditions ) const;
	QList<Template::IncludePath>	 getIncludePathsMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<Template::LibraryPath>	 getLibraryPathsMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<Template::FrameworkPath>	 getFrameworkPathsMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<Template::StaticLibrary>	 getStaticLibrariesMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<Template::DynamicLibrary>	 getDynamicLibrariesMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<Template::BuildSetting>	 getBuildSettingsMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<Template::PreprocessorDefine> getPreprocessorDefinesMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<Template::OutputExtension> getOutputExtensionsMatchingConditions( const QMap<QString,QString> &conditions ) const;

	// takes ownership of childGen
	void			addGenerator( GeneratorBase *childGen );
	void			addCinderBlock( const CinderBlock &block ) { mCinderBlocks.push_back( CinderBlockRef( new CinderBlock( block ) ) ); }
	void			generate( bool setupGit );

	QString         getProjectName() const;
	void            setProjectName( const QString &projName );

	QString         getNamePrefix() const;
	void            setNamePrefix( const QString &namePrefix );

	QString         getBaseLocation() const;
	void            setBaseLocation( const QString &baseLocation );

	QDir			getOutputDir() const { return QDir( getBaseLocation() + "/" + getProjectName() ); }

	QString			getWinRelCinderPath( const QString &relativeTo ) const { return getRelCinderPath( relativeTo ).replace( "/", "\\" ); }
	QString         getMacRelCinderPath( const QString &relativeTo ) const;
	QString			getCinderAbsolutePath() const { return mAbsCinderPath; }
	void            setCinderAbsolutePath( const QString &absCinderPath );

	QString			createDirectory( QString relPath ) const;
	QString			getAbsolutePath( QString relPath ) const;

  private:
	template<Template::File::Type FILE_TYPE>
	QList<Template::File> getFileTypeMatchingConditions( const QList<QMap<QString,QString> > &conditions, bool getCopyOnly ) const;

	bool			prepareGenerate();
	void			writeResourcesHeader( const QList<QMap<QString,QString> > &conditions ) const;
	void			copyAssets( const QList<QMap<QString,QString> > &conditions ) const;
	void			copyBareFiles( const QList<QMap<QString,QString> > &conditions ) const;
	QString         getRelCinderPath( const QString &relativeTo ) const;
	bool			setupGitRepo( const QString &dirPath );
	bool			initialCommitToGitRepo( const QString &dirPath );

    ProjectTemplate					mProjectTmpl;
    QSharedPointer<ProjectTemplate>	mChildTemplate;
    
	QString         mProjectName;
	QString         mNamePrefix;
	QString         mBaseLocation;
	QString         mAbsCinderPath;

	QList<GeneratorBaseRef>		mChildGenerators;
	QList<CinderBlockRef>		mCinderBlocks;
};
