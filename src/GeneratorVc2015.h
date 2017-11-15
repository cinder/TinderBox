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

#include <QString>
#include <QFileInfo>

#include "ProjectTemplate.h"
#include "GeneratorVcBase.h"

class GeneratorVc2015 : public GeneratorVcBase {
  public:
	struct Options;

	GeneratorVc2015( const Options &options )
		: GeneratorVcBase( "vc2015" ), mOptions( options )
	{}

	struct Options {
	  public:
		void		enableWin32( bool enable ) { mEnableWin32 = enable; }
		void		enableX64( bool enable ) { mEnableX64 = enable; }
		void		enableDesktopGl( bool enable ) { mEnableDesktopGl = enable; }
		void		enableAngle( bool enable ) { mEnableAngle = enable; }

		bool		mEnableWin32, mEnableX64, mEnableDesktopGl, mEnableAngle;
	};

	GeneratorConditions							getBaseConditions() const;
	virtual QString                             getCompiler() const;
	std::vector<VcProj::ProjectConfiguration>	getPlatformConfigurations() const override;
	virtual bool                                getSlnDeploy() const;
	virtual bool                                getUseRcFile() const { return true; }

	virtual VcProjRef	createVcProj( const QString &vcProj, const QString &vcProjFilters );

	Options		mOptions;
};
