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

#include "GeneratorVc2019.h"
#include "ProjectTemplateManager.h"
#include "Util.h"
#include "Vc2015Proj.h"

#include <fstream>

GeneratorConditions GeneratorVc2019::getBaseConditions() const
{
	QMap<QString,QString> conditions;
	conditions["compiler"] = "vc2015";
	conditions["os"] = "msw";
	return GeneratorConditions( conditions );
}

VcProjRef GeneratorVc2019::createVcProj( const QString &VcProj, const QString &VcProjFilters )
{
    return Vc2015Proj::createFromString( VcProj, VcProjFilters );
}

std::vector<VcProj::ProjectConfiguration> GeneratorVc2019::getPlatformConfigurations() const
{
    std::vector<VcProj::ProjectConfiguration> result;

	// Win32 GL
	if( mOptions.mEnableWin32 && mOptions.mEnableDesktopGl ) {
		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug" ), QString::fromUtf8( "Win32" ) ) );
		{auto conditions = getBaseConditions(); conditions.setCondition( "arch", "i386" ); conditions.setCondition( "config", "debug" );
		result.back().setConditions( conditions );}

		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release" ), QString::fromUtf8( "Win32" ) ) );
		{auto conditions = getBaseConditions(); conditions.setCondition( "arch", "i386" ); conditions.setCondition( "config", "release" );
		result.back().setConditions( conditions );}
	}

	// Win32 ANGLE
	if( mOptions.mEnableWin32 && mOptions.mEnableAngle ) {
		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug_ANGLE" ), QString::fromUtf8( "Win32" ) ) );
		{auto conditions = getBaseConditions(); conditions.setCondition( "arch", "i386" ); conditions.setCondition( "config", "debug" );
		result.back().setConditions( conditions );}

		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release_ANGLE" ), QString::fromUtf8( "Win32" ) ) );
		{auto conditions = getBaseConditions(); conditions.setCondition( "arch", "i386" ); conditions.setCondition( "config", "release" );
		result.back().setConditions( conditions );}
	}

	// x64 GL
	if( mOptions.mEnableX64 && mOptions.mEnableDesktopGl ) {
		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug" ), QString::fromUtf8( "x64" ) ) );
		{auto conditions = getBaseConditions(); conditions.setCondition( "arch", "x86_64" ); conditions.setCondition( "config", "debug" );
		result.back().setConditions( conditions );}

		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release" ), QString::fromUtf8( "x64" ) ) );
		{auto conditions = getBaseConditions(); conditions.setCondition( "arch", "x86_64" ); conditions.setCondition( "config", "release" );
		result.back().setConditions( conditions );}
	}

    // x64 ANGLE
    if( mOptions.mEnableWin32 && mOptions.mEnableAngle ) {
        result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug_ANGLE" ), QString::fromUtf8( "x64" ) ) );
		{auto conditions = getBaseConditions(); conditions.setCondition( "arch", "x86_64" ); conditions.setCondition( "config", "debug" );
        result.back().setConditions( conditions );}

        result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release_ANGLE" ), QString::fromUtf8( "x64" ) ) );
		{auto conditions = getBaseConditions(); conditions.setCondition( "arch", "x86_64" ); conditions.setCondition( "config", "release" );
        result.back().setConditions( conditions );}
    }

	return result;
}

bool GeneratorVc2019::getSlnDeploy() const
{
	return false;
}
