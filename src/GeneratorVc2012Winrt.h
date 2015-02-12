#pragma once

#include <QString>
#include <QFileInfo>

#include "ProjectTemplate.h"
#include "GeneratorVcBase.h"

class GeneratorVc2012Winrt : public GeneratorVcBase {
  public:
	GeneratorVc2012Winrt()
		: GeneratorVcBase( "vc2012_winrt" )
	{}

    virtual QMap<QString,QString>                       getConditions() const;
    virtual QString                                     getCompiler() const;
    virtual std::vector<VcProj::ProjectConfiguration>	getPlatformConfigurations() const override;
    virtual bool                                        getSlnDeploy() const;
    virtual bool                                        getUseRcFile() const { return false; }

	virtual VcProjRef	createVcProj( const QString &vcProj, const QString &vcProjFilters );
};
