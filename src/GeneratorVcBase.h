#pragma once

#include <QString>
#include <QFileInfo>

#include "TinderBox.h"
#include "ProjectTemplate.h"
#include "GeneratorBase.h"
#include "Instancer.h"
#include "VcProj.h"

class GeneratorVcBase : public GeneratorBase {
  public:
	GeneratorVcBase( const QString &foundationName );

	virtual QMap<QString,QString>	getConditions() const = 0;
	virtual QString					getCompiler() const = 0;
	virtual std::vector<QString>	getArchitectures() const = 0;
	virtual bool					getSlnDeploy() const = 0;
	virtual bool					getUseRcFile() const = 0;

	virtual void					generate( Instancer *master );
  protected:
	virtual VcProjRef				createVcProj( const QString &vcProj, const QString &vcProjFilters ) = 0;

	void	setupIncludePaths( VcProjRef proj, Instancer *master, QMap<QString,QString> &conditions, const QString &config, const QString &absPath, const QString &cinderPath );
	void	setupLibraryPaths( VcProjRef proj, Instancer *master, QMap<QString,QString> &conditions, const QString &config, const QString &absPath, const QString &cinderPath );
	void	setupPreprocessorDefines( VcProjRef proj, Instancer *master, QMap<QString,QString> &conditions, const QString &config );

	QString		mFoundationName;
};
