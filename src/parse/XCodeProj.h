#pragma once

#include "PList.h"
#include "TinderBox.h"

#include <QSharedPointer>
#include <iostream>

class XCodeProjExc : public TinderBoxExc {
  public:
	XCodeProjExc( const QString &excMsg ) : TinderBoxExc( excMsg ) {}
};

typedef QSharedPointer<class XCodeProj>		XCodeProjRef;
class XCodeProj
{
  protected:
	class PbxGroup;
	class PbxBuildPhase;
	class PbxNativeTarget;
	class XcBuildConfiguration;
	class XcConfigurationList;
	class PbxProject;

  public:
	XCodeProj( QSharedPointer<PList> plist );

	static XCodeProjRef  createFromFilePath( QString path );
	static XCodeProjRef  createFromString( const QString &s );

	static QString	getFileType( const QString &fileSystemPath, const QString &extensionOverride = QString() );

	// high-level manipulation functions
    void	addSourceFile( const QString &fileSystemPath, const QString &virtualPath, const QString &compileAsExtension );
	void	addHeaderFile( const QString &fileSystemPath, const QString &virtualPath );
	void	addResource( const QString &fileSystemPath, const QString &virtualPath, bool excludeFromBuild );
	void	addStaticLibrary( const QString &config, const QString &sdk, const QString &path );
	void	addDynamicLibrary( const QString &config, const QString &sdk, const QString &path );
	void	addSystemHeaderPath( const QString &config, const QString &sdk, const QString &path );
	void	addUserHeaderPath( const QString &config, const QString &sdk, const QString &path );
	void	addLibraryPath( const QString &config, const QString &sdk, const QString &path );
	void	addFrameworkPath( const QString &config, const QString &sdk, const QString &path );
	void	addFramework( const QString &fileSystemPath, const QString &virtualPath, bool absolute, bool sdkRelative );
	void	addBuildCopy( const QString &fileSystemPath, const QString &virtualPath, bool absolute, bool sdkRelative, const QString &destinationBuildPhase );
	void	addPreprocessorDefine( const QString &config, const QString &sdk, const QString &value ); // adds to target
	void	setBuildSetting( const QString &configuration, const QString &sdk, const QString &settingName, const QString &settingValue, bool favorTarget );
	
	// low-level manipulation functions
	PList::DictionaryRef	getTargetBuildSettings( const QString &configuration );
	PList::DictionaryRef	getProjectBuildSettings( const QString &configurationName );
	QList<PList::ItemRef>	findDominantBuildSettings( QString configuration, QString sdk, QString settingName, PList::DictionaryRef *resultParentDictionary );
	void					appendBuildSettingArray( const QString &configuration, const QString &sdk, const QString &settingName, const QString &settingValue, bool favorTarget );
	void					createBuildSettingArray( PList::DictionaryRef settings, const QString &sdk, const QString &settingName, const QString &settingValue );

	void	print( std::ostream &os );
	void	setupLineComments();

	QSharedPointer<PbxBuildPhase>			findBuildPhase( const QString &isa );
	QSharedPointer<PbxBuildPhase>			createOrFindCopyFilesBuildPhase( int dstSubfolderSpec );
	void									addBuildPhase( QSharedPointer<PbxBuildPhase> buildPhase );
	QSharedPointer<XcBuildConfiguration>	findBuildConfiguration( QSharedPointer<XcConfigurationList> configList, QString name );

	// Heuristic-based search for group best qualified to hold a source file
	QSharedPointer<PbxGroup>	findSourcesGroup();
	// Heuristic-based search for group best qualified to hold a header file
	QSharedPointer<PbxGroup>	findHeadersGroup();
	// Heuristic-based search for group best qualified to hold a resource file
	QSharedPointer<PbxGroup>	findResourcesGroup();
	// Heuristic-based search for group best qualified to hold a Framework
	QSharedPointer<PbxGroup>	findFrameworksGroup();
    // recursively creates groups necessary to achieve 'virtualPath'. Assumes last item is a filename and ignores it
    QSharedPointer<PbxGroup>    createAndFindGroupsPath( QSharedPointer<PbxGroup> relativeTo, const QString &virtualPath );
	// Parent group for the whole project
	QSharedPointer<PbxGroup>	getMainGroup() { return mMainGroup; }

	// Returns "the" target, which is currently defined as the first target
	QSharedPointer<PbxNativeTarget>		getTarget() { return mPbxNativeTargets.front(); }
	// Returns the project
	QSharedPointer<PbxProject>			getProject() { return mPbxProjects.front(); }

	QString		generateUniqueTag() const;

  protected:
	class PbxItem {
	  public:
		PbxItem( QSharedPointer<PList::Dictionary> plistItem );
		virtual ~PbxItem() {}

		QSharedPointer<PList::Dictionary>		getPListItem() { return mPListItem; }
		const QSharedPointer<PList::Dictionary>	getPListItem() const { return mPListItem; }

		const QString&		getTag() const { return mPListItem->getTag(); }
		const QString&		getIsa() const { return mPListItem->getValue( "isa", "" ); }

		const QString	operator[]( const QString &qs ) const { if( mPListItem->contains( qs ) ) return (*mPListItem)[qs]; else return QString(); }
		QString&	operator[]( const QString &qs ) { return (*mPListItem)[qs]; }

		QSharedPointer<PList::Dictionary>		mPListItem;
	};

	class PbxFileReference : public PbxItem {
	  public:
		PbxFileReference( const QString &tag );
		PbxFileReference( QSharedPointer<PList::Dictionary> plistItem );

		QString				getPathFileName() const { return getPath().section( '/', -1 ); }
		const QString&		getPath() const { return mPListItem->getValue( "path", "" ); }

		void		print( std::ostream &os ) { os << "path=" << qPrintable(getPath()); }
	};

	class PbxBuildFile : public PbxItem {
	  public:
		PbxBuildFile( QSharedPointer<PList::Dictionary> plistItem );
		PbxBuildFile( const QString &tag, QSharedPointer<PbxFileReference> fileRef );

		void	setupFileRef( XCodeProj *proj );

		QString				getReferencedFileName() const;
		const QString&		getFileRefString() const { return mPListItem->getValue( "fileRef", "" ); }

		QSharedPointer<PbxFileReference>		mFileRef;
	};

	class PbxBuildRule : public PbxItem {
	  public:
		PbxBuildRule( QSharedPointer<PList::Dictionary> plistItem );
	};

	class PbxBuildPhase : public PbxItem {
	  public:
		PbxBuildPhase( const QString &tag, const QString &isa );
		PbxBuildPhase( QSharedPointer<PList::Dictionary> plistItem );

		QString		getHumanReadableName() const;

		void		add( PbxBuildFile &buildFile );
		void		setupBuildFiles( XCodeProj *proj );

		QList<QSharedPointer<PbxBuildFile> >			mBuildFiles;
	};

	class PbxGroup : public PbxItem {
	  public:
		PbxGroup( QSharedPointer<PList::Dictionary> plistItem );
        PbxGroup( const QString &tag, const QString &name );

        static QSharedPointer<PbxGroup> create( const QString &tag, const QString &name ) { return QSharedPointer<PbxGroup>( new PbxGroup( tag, name ) ); }

		const QString&	getName() const { return mName; }

		void		setupFileReferences( XCodeProj *proj );
		void		print( std::ostream &os );

        void                    add( QSharedPointer<PbxFileReference> fileRef );
        void                    add( QSharedPointer<PbxGroup> group );
        QSharedPointer<PbxItem> findChildImpl( const QString &name ) const;
        template<typename T>
        QSharedPointer<T>			findChild( const QString &tag ) const { return findChildImpl( tag ).dynamicCast<T>(); }
		QSharedPointer<PbxGroup>	findChildGroup( const QString &name ) const;

		QList<QSharedPointer<PbxItem> >		mChildren;
		QString								mName;
	};

	class PbxNativeTarget : public PbxItem {
	  public:
		PbxNativeTarget( QSharedPointer<PList::Dictionary> plistItem );

		void		setupReferences( XCodeProj *proj );
		void		print( std::ostream &os );

		QList<QSharedPointer<PbxBuildPhase> >		mBuildPhases;
		QSharedPointer<PbxFileReference>			mProductReference;
	};

	class XcBuildConfiguration : public PbxItem {
	  public:
		XcBuildConfiguration( QSharedPointer<PList::Dictionary> plistItem );

		QString		getName() const { return mName; }

		QString		mName;
	};

	class XcConfigurationList : public PbxItem {
	  public:
		XcConfigurationList( QSharedPointer<PList::Dictionary> plistItem );
		void		setupReferences( XCodeProj *proj );

		QList<QString>	getConfigurationTags();

		QList<QSharedPointer<XcBuildConfiguration> >			mBuildConfigurations;
	};

	class PbxProject : public PbxItem {
	  public:
		PbxProject( QSharedPointer<PList::Dictionary> plistItem );

		QSharedPointer<PbxGroup>		getMainGroup() { return mMainGroup; }

		void		setupReferences( XCodeProj *proj );
		void		print( std::ostream &os );

		QList<QSharedPointer<PbxNativeTarget> >			mTargets;
		QSharedPointer<PbxGroup>							mMainGroup;
	};


	QSharedPointer<PbxItem>		findItemGeneric( const QString &tag );
	template<typename T>
	QSharedPointer<T>			findItem( const QString &tag ) {
		QSharedPointer<PbxItem> item = findItemGeneric( tag );
		if( item ) {
			return item.dynamicCast<T>();
		}
		else
			return QSharedPointer<T>(); // failure
	}

	// finds an existing PbxFileReference for a given filesystem path
	QSharedPointer<XCodeProj::PbxFileReference> findExistingFileReference( const QString &fileSystemPath );
	// finds an existing PbxFileReference for a given fileType and fileName
	QSharedPointer<PbxFileReference>	findExistingFileReference( const QString &fileType, const QString &fileName );

	QSharedPointer<PList>								mPList;
	QList<QSharedPointer<PbxBuildFile> >				mPbxBuildFiles;
	QList<QSharedPointer<PbxFileReference> >			mPbxFileReferences;
	QList<QSharedPointer<PbxBuildPhase> >				mPbxBuildPhases;
	QList<QSharedPointer<PbxBuildRule> >				mPbxBuildRules;
	QList<QSharedPointer<PbxGroup> >					mPbxGroups;
	QList<QSharedPointer<PbxNativeTarget> >			mPbxNativeTargets;
	QList<QSharedPointer<XcBuildConfiguration> >		mXcBuildConfigurations;
	QList<QSharedPointer<XcConfigurationList> >		mXcConfigurationLists;
	QList<QSharedPointer<PbxProject> >					mPbxProjects;
	QList<QSharedPointer<PbxItem> >						mUnhandledItems;
	QSharedPointer<PbxGroup>							mMainGroup;
};
