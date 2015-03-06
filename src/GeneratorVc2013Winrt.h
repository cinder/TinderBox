#pragma once

#include <QString>
#include <QFileInfo>

#include "ProjectTemplate.h"
#include "GeneratorVcBase.h"

class GeneratorVc2013WinRt : public GeneratorVcBase {
  public:
	struct Options;

	GeneratorVc2013WinRt( const Options & options )
		: GeneratorVcBase( "vc2013_winrt" ), mOptions( options )
	{}

	struct Options {
	  public:
		void		enableWin32( bool enable ) { mEnableWin32 = enable; }
		void		enableX64( bool enable ) { mEnableX64 = enable; }
		void		enableArm( bool enable ) { mEnableArm = enable; }

		bool		mEnableWin32, mEnableX64, mEnableArm;
	};


    virtual QMap<QString,QString>                       getConditions() const;
    virtual QString                                     getCompiler() const;
    virtual std::vector<VcProj::ProjectConfiguration>	getPlatformConfigurations() const override;
    virtual bool                                        getSlnDeploy() const;
    virtual bool                                        getUseRcFile() const { return false; }

	virtual VcProjRef	createVcProj( const QString &vcProj, const QString &vcProjFilters );

  private:
	Options				mOptions;
};
