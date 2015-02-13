#pragma once

#include <QString>
#include <QFileInfo>

#include "ProjectTemplate.h"
#include "GeneratorVcBase.h"

class GeneratorVc2013 : public GeneratorVcBase {
  public:
	struct Options;

	GeneratorVc2013( const Options &options )
		: GeneratorVcBase( "vc2013" ), mOptions( options )
	{}

	struct Options {
	  public:
		void		enableWin32( bool enable ) { mEnableWin32 = enable; }
		void		enableX64( bool enable ) { mEnableX64 = enable; }
		void		enableDesktopGl( bool enable ) { mEnableDesktopGl = enable; }
		void		enableAngle( bool enable ) { mEnableAngle = enable; }

		bool		mEnableWin32, mEnableX64, mEnableDesktopGl, mEnableAngle;
	};

	virtual QMap<QString,QString>               getConditions() const;
	virtual QString                             getCompiler() const;
	std::vector<VcProj::ProjectConfiguration>	getPlatformConfigurations() const override;
	virtual bool                                getSlnDeploy() const;
	virtual bool                                getUseRcFile() const { return true; }

	virtual VcProjRef	createVcProj( const QString &vcProj, const QString &vcProjFilters );

	Options		mOptions;
};
