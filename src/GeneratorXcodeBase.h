#pragma once

#include <QString>
#include <QFileInfo>

#include "TinderBox.h"
#include "ProjectTemplate.h"
#include "GeneratorBase.h"
#include "Instancer.h"
#include "XCodeProj.h"

class GeneratorXcodeBase : public GeneratorBase {
  public:
	virtual QMap<QString,QString>	getConditions() const = 0;
	virtual void					generate( Instancer *master );
	virtual QString					getRootFolderName() const = 0;
	
	virtual QList<QString>			getSdks() const = 0;
	
  protected:
	void	setupIncludePaths( XCodeProjRef xcodeProj, Instancer *master, const QMap<QString,QString> &conditions, const QString &config,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupLibraryPaths( XCodeProjRef xcodeProj, Instancer *master, const QMap<QString,QString> &conditions, const QString &config,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupFrameworkPaths( XCodeProjRef xcodeProj, Instancer *master, const QMap<QString,QString> &conditions, const QString &config,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupStaticLibaries( XCodeProjRef xcodeProj, Instancer *master, const QMap<QString,QString> &conditions, const QString &config,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupDynamicLibaries( XCodeProjRef xcodeProj, Instancer *master, const QMap<QString,QString> &conditions, const QString &config,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupPreprocessorDefines( XCodeProjRef xcodeProj, Instancer *master, const QMap<QString,QString> &conditions, const QString &config );
	void	setupBuildSettings( XCodeProjRef xcodeProj, Instancer *master, const QMap<QString,QString> &conditions, const QString &config );
	void	setupOutputExtension( XCodeProjRef xcodeProj, Instancer *master, const QMap<QString,QString> &conditions, const QString &config );
								
	template<typename T>
	QList<QString> getUniqueSdks( Instancer *master, QList<T> (Instancer::*fn)( const QMap<QString,QString> & ) const, const QMap<QString,QString> &conditions );
};
