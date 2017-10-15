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

#include <QMap>
#include <QList>
#include <QDir>

#include "TinderBox.h"
#include "ErrorList.h"

class Template {
  public:
    class File;
    class IncludePath;
    
    class Item {
      public:
		virtual ~Item() {}

		Item( const QString &parentPath, const QString &inputPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions );

		bool		conditionsMatch( const QMap<QString,QString> &conditions ) const;
		
		QString			getAbsoluteInputPath() const { return mInputAbsolutePath; }
		QString			getRelativeInputPath() const { return mInputRelativePath; }
		QString			getAbsoluteOutputPath() const;
		QString			getMacOutputPathRelativeTo( const QString &relativeTo, const QString &cinderPath ) const {
			return getOutputPathRelativeTo( relativeTo, cinderPath ); }
		QString			getWinOutputPathRelativeTo( const QString &relativeTo, const QString &cinderPath ) const {
			return getOutputPathRelativeTo( relativeTo, cinderPath ).replace( "/", "\\" ); }

		// Is output an absolute path? Always true if sdk-relative
		bool			isOutputAbsolute() const { return mOutputIsAbsolute; }
		bool			isOutputSdkRelative() const { return mOutputIsSdkRelative; }
		bool			isOutputCinderRelative() const { return mOutputIsCinderRelative; }
		bool			isOutputBuildExcluded() const { return mBuildExclude; }

		// Should the instancer copy this file?
		bool			shouldCopy() const { return ( ! mOutputIsAbsolute ) && ( ! mOutputIsCinderRelative ); }

        virtual void	setOutputPath( const QString &outputPath, const QString &/*replaceName*/, const QString &cinderPath );

		bool			operator==( const Item &rhs ) const { return
			mInputRelativePath == rhs.mInputRelativePath && mInputAbsolutePath == rhs.mInputAbsolutePath &&
			mOutputIsAbsolute == rhs.mOutputIsAbsolute && mOutputIsSdkRelative == rhs.mOutputIsSdkRelative
			&& mOutputIsCinderRelative == rhs.mOutputIsCinderRelative && mBuildExclude == rhs.mBuildExclude; }

		static QStringList	knownAttributes();

	  protected:
		QString			getOutputPathRelativeTo( const QString &relativeTo, const QString &cinderPath ) const;

		QMap<QString,QString>		mConditions;
		QString						mInputAbsolutePath;
		QString						mInputRelativePath;
		QString						mOutputAbsolutePath;
		bool						mOutputIsAbsolute, mOutputIsSdkRelative, mOutputIsCinderRelative;
		bool						mBuildExclude;
		
		friend class Template;
    };

	class File : public Item {
	  public:
		typedef enum { SOURCE, HEADER, RESOURCE, ASSET, FRAMEWORK, DIRECTORY, BUILD_COPY, FILE } Type;
	  
		File( const QString &parentPath, const QString &inputPath, const pugi::xml_node &dom, Type type, const QMap<QString,QString> &conditions, ErrorList *errors );

      public:
		File::Type		getType() const { return mType; }
		
        QString         getVirtualPath() const { return mVirtualPath; }
        void			setVirtualPath( const QString &virtualPath ) { mVirtualPath = virtualPath; }
		bool			getReplaceName() const { return mReplaceName; }
		bool			getReplaceContents() const { return mReplaceContents; }
		QString			getCompileAs() const { return mCompileAs; }
		void			setInputPath( const QString &parentPath, const QString &inputPath );

		// header-specific
		bool			isResourceHeader() const { return mResourceHeader; }
		bool			isPch() const { return mPch; }

		// resource-specific
		QString			getResourceName() const { return mResourceName; }
		QString			getResourceType() const { return mResourceType; }
		int				getResourceId() const { return mResourceId; }
		void			setResourceId( int id ) { mResourceId = id; }

		// build-copy-specific
		QString			getBuildCopyDestination() const { return mBuildCopyDestination; }

        virtual void	setOutputPath( const QString &outputPath, const QString &replaceName, const QString &cinderPath, const QString &replaceProjDir );
        QString			getMacOutputPath( const QString &outputPath, const QString &replacePrefix, const QString &cinderPath, const QString &replaceProjDir ) const;

	  protected:
		Type			mType;	  
		QString			mVirtualPath, mCompileAs;
		bool			mReplaceContents, mReplaceName;
		// header-specific
		bool			mResourceHeader; // whether this file is to be used as Resources.h
		bool			mPch; // whether this file is used as a precompiled header
		// resource-specific
		QString			mResourceName;
		QString			mResourceType;
		int				mResourceId;
		// build-copy-specific
		QString			mBuildCopyDestination;
	};

	class IncludePath : public Item {
	  public:
		IncludePath( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors );
		
		bool	isSystem() const { return mSystem; }
		
		bool	operator==( const IncludePath &rhs ) const { return Item::operator==(rhs) && mSystem == rhs.mSystem; }
	  protected:
		bool		mSystem;
	};

	class LibraryPath : public Item {
	  public:
		LibraryPath( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors );
	};

	class FrameworkPath : public Item {
	  public:
		FrameworkPath( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors );
	};
	
	class StaticLibrary : public Item {
	  public:
		StaticLibrary( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors );		
	};

	class DynamicLibrary : public Item {
	  public:
		DynamicLibrary( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors );
	};

	class BuildSetting : public Item {
	  public:
		BuildSetting( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors );
		
		QString		getKey() const { return mKey; }
		QString		getValue() const { return mValue; }
		
		QString		mKey, mValue;
	};

	class PreprocessorDefine : public Item {
	  public:
		PreprocessorDefine( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors );

		QString		getValue() const { return mValue; }

		QString		mValue;
	};

	class OutputExtension : public Item {
	  public:
		OutputExtension( const QString &parentPath, const pugi::xml_node &dom, const QMap<QString,QString> &conditions, ErrorList *errors );

		QString		getValue() const { return mValue; }

		QString		mValue;
	};

	Template() {}
	Template( const QString &parentPath, const pugi::xml_node &doc, ErrorList *errors );
	virtual ~Template() {}

	QString		getName() const { return mName; }
	QString		getId() const { return mId; }
	
	void        setName( const QString &name ) { mName = name; }

	const QList<QString>&	getRequires() const { return mRequires; }

	// for copy submodule type
    void			setOutputPath( const QString &outputPath, const QString &replaceName, const QString &cinderPath, const QString &replaceProjDir );
	// for relative/submodule type; output == input
	void			setOutputPathToInput();
	// prepends 'virtualPath' to all items' virtual paths
	void			setupVirtualPaths( const QString &virtualPath );

	bool			supportsConditions( const QMap<QString,QString> &conditions ) const;
	bool			isCore() const { return mCore; }

	void			instantiateFilesMatchingConditions( const QList<QMap<QString,QString> > &conditionsList, bool overwriteExisting ) const;
	QList<File>		getFilesMatchingConditions( const QList<QMap<QString,QString> > &conditionsList ) const;
	QList<File>		getFilesMatchingConditions( const QMap<QString,QString> &conditions ) const;

	QList<IncludePath>		getIncludePathsMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<LibraryPath>		getLibraryPathsMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<FrameworkPath>	getFrameworkPathsMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<StaticLibrary>	getStaticLibrariesMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<DynamicLibrary>	getDynamicLibrariesMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<BuildSetting>		getBuildSettingsMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<PreprocessorDefine>	getPreprocessorDefinesMatchingConditions( const QMap<QString,QString> &conditions ) const;
	QList<OutputExtension>	getOutputExtensionsMatchingConditions( const QMap<QString,QString> &conditions ) const;

	QString			getOutputPath() const { return mOutputPath; }
	QString			getParentPath() const { return mParentPath; }

  protected:
	void		processFilePattern( const QString &parentPath, const pugi::xml_node &dom, File::Type type, const QMap<QString,QString> &conditions, ErrorList *errors );
	template<typename T>
	QList<T>	getItemsMatchingConditions( const QList<T> &list, const QMap<QString,QString> &conditions ) const;
	void		parseGroup( const pugi::xml_node &node, QMap<QString,QString> conditions, ErrorList *errors );
	void		parseSupports( const pugi::xml_node &node, ErrorList *errors );

	QString					mParentPath;
	QString					mOutputPath, mReplacementPrefix, mCinderPath;
	
	QString					mName, mId;
	QList<File>				mFiles;
	QList<IncludePath>		mIncludePaths;
	QList<LibraryPath>		mLibraryPaths;
	QList<FrameworkPath>	mFrameworkPaths;
	QList<StaticLibrary>	mStaticLibraries;
	QList<DynamicLibrary>	mDynamicLibraries;
	QList<BuildSetting>		mBuildSettings;
	QList<PreprocessorDefine>	mPreprocessorDefines;
	QList<OutputExtension>		mOutputExtensions;

	bool							mCore;
	QList<QString>					mRequires;
	QList<QMap<QString,QString> >	mSupports;
};

typedef QSharedPointer<Template> TemplateRef;
