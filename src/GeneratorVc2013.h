#pragma once

#include <QString>
#include <QFileInfo>

#include "ProjectTemplate.h"
#include "GeneratorVcBase.h"

class GeneratorVc2013 : public GeneratorVcBase {
  public:
	GeneratorVc2013()
		: GeneratorVcBase( "vc2013" )
	{}

	virtual QMap<QString,QString>	getConditions() const;
	virtual QString					getCompiler() const;
	virtual std::vector<QString>	getArchitectures() const;
	virtual bool					getSlnDeploy() const;
	virtual bool					getUseRcFile() const { return true; }

	virtual VcProjRef	createVcProj( const QString &vcProj, const QString &vcProjFilters );
};
