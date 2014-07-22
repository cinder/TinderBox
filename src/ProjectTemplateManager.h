#pragma once

#include "TinderBox.h"
#include "ErrorList.h"
#include "ProjectTemplate.h"

#include <QDir>

class ProjectTemplateManager {
  public:
	static const QList<ProjectTemplate>&	getTemplates() { return inst()->mTemplates; }
	static QStringList						getProjectTemplateNames() { return inst()->getProjectTemplateNamesImpl(); }
	
	static void								clear() { inst()->mTemplates.clear(); }
	static void								setCinderDir( QDir cinderDir, ErrorList *errorList );

	static QString							getFoundationPath( QString relativePath ) { return inst()->getFoundationPathImpl( relativePath ); }
	static const ProjectTemplate&			getProjectById( const QString &projectId ) { return inst()->getProjectByIdImpl( projectId ); }

	static ProjectTemplateManager*		inst();

	class ProjectNotFoundExc : virtual public std::exception {
	};

  private:
	ProjectTemplateManager();

	QStringList			getProjectTemplateNamesImpl() const;
	QString				getFoundationPathImpl( QString relativePath );
	void				scanImpl( const QDir &cinderDir, QDir dir, ErrorList *errorList );
	
	const ProjectTemplate&	getProjectByIdImpl( const QString &projectId );

	QDir						mCinderDir;
	QList<ProjectTemplate>		mTemplates;
};
