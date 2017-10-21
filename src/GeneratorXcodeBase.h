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
#include "XCodeProj.h"

class GeneratorXcodeBase : public GeneratorBase {
  public:
	virtual std::vector<GeneratorConditions>	getConditions() const = 0;
	virtual void								generate( Instancer *instancer );
	virtual QString								getRootFolderName() const = 0;
	
	virtual QList<QString>						getSdks() const = 0;
	
  protected:
	void	setupIncludePaths( XCodeProjRef xcodeProj, Instancer *instancer, const GeneratorConditions &conditions,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupLibraryPaths( XCodeProjRef xcodeProj, Instancer *instancer, const GeneratorConditions &conditions,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupFrameworkPaths( XCodeProjRef xcodeProj, Instancer *instancer, const GeneratorConditions &conditions,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupStaticLibaries( XCodeProjRef xcodeProj, Instancer *instancer, const GeneratorConditions &conditions,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupDynamicLibaries( XCodeProjRef xcodeProj, Instancer *instancer, const GeneratorConditions &conditions,
								const QString &xcodeAbsPath, const QString &cinderPath );
	void	setupPreprocessorDefines( XCodeProjRef xcodeProj, Instancer *instancer, const GeneratorConditions &conditions );
	void	setupBuildSettings( XCodeProjRef xcodeProj, Instancer *instancer, const GeneratorConditions &conditions );
	void	setupOutputExtension( XCodeProjRef xcodeProj, Instancer *instancer, const GeneratorConditions &conditions );
								
	template<typename T>
	QList<QString> getUniqueSdks( Instancer *instancer, QList<T> (Instancer::*fn)( const GeneratorConditions& ) const, const GeneratorConditions &conditions );
};
