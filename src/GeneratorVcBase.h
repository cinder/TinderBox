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

#include "TinderBox.h"
#include "ProjectTemplate.h"
#include "GeneratorBase.h"
#include "Instancer.h"
#include "VcProj.h"

class GeneratorVcBase : public GeneratorBase {
  public:
	GeneratorVcBase( const QString &foundationName );

	virtual QMap<QString,QString>                       getConditions() const = 0;
	virtual QString                                     getCompiler() const = 0;
	virtual std::vector<VcProj::ProjectConfiguration>	getPlatformConfigurations() const = 0;
	virtual bool                                        getSlnDeploy() const = 0;
	virtual bool                                        getUseRcFile() const = 0;

	virtual void					generate( Instancer *master );
  protected:
	virtual VcProjRef				createVcProj( const QString &vcProj, const QString &vcProjFilters ) = 0;

	void	setupIncludePaths( VcProjRef proj, Instancer *master, const VcProj::ProjectConfiguration &config, const QString &absPath, const QString &cinderPath );
	void	setupLibraryPaths( VcProjRef proj, Instancer *master, const VcProj::ProjectConfiguration &config, const QString &absPath, const QString &cinderPath );
	void	setupPreprocessorDefines( VcProjRef proj, Instancer *master, const VcProj::ProjectConfiguration &config );

	QString		mFoundationName;
};
