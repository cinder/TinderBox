#include "GeneratorVc2013.h"
#include "ProjectTemplateManager.h"
#include "Util.h"
#include "Vc2013Proj.h"

#include <fstream>

QMap<QString,QString> GeneratorVc2013::getConditions() const
{
	QMap<QString,QString> conditions;
	conditions["compiler"] = "vc2013";
	conditions["os"] = "msw";
	return conditions;
}

QString GeneratorVc2013::getCompiler() const
{
	return QString( "120" );
}

VcProjRef GeneratorVc2013::createVcProj( const QString &VcProj, const QString &VcProjFilters )
{
    return Vc2013Proj::createFromString( VcProj, VcProjFilters );
}

std::vector<VcProj::ProjectConfiguration> GeneratorVc2013::getPlatformConfigurations() const
{
    std::vector<VcProj::ProjectConfiguration> result;

	// Win32 GL
	if( mOptions.mEnableWin32 && mOptions.mEnableDesktopGl ) {
		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug" ), QString::fromUtf8( "Win32" ) ) );
		{auto conditions = getConditions(); conditions["arch"] = "i386"; conditions["config"] = "debug";
		result.back().setConditions( conditions );}

		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release" ), QString::fromUtf8( "Win32" ) ) );
		{auto conditions = getConditions(); conditions["arch"] = "i386"; conditions["config"] = "release";
		result.back().setConditions( conditions );}
	}

	// Win32 ANGLE
	if( mOptions.mEnableWin32 && mOptions.mEnableAngle ) {
		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug_ANGLE" ), QString::fromUtf8( "Win32" ) ) );
		{auto conditions = getConditions(); conditions["arch"] = "i386"; conditions["config"] = "debug";
		result.back().setConditions( conditions );}

		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release_ANGLE" ), QString::fromUtf8( "Win32" ) ) );
		{auto conditions = getConditions(); conditions["arch"] = "i386"; conditions["config"] = "release";
		result.back().setConditions( conditions );}
	}

	// x64 GL
	if( mOptions.mEnableX64 && mOptions.mEnableDesktopGl ) {
		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug" ), QString::fromUtf8( "x64" ) ) );
		{auto conditions = getConditions(); conditions["arch"] = "x86_64"; conditions["config"] = "debug";
		result.back().setConditions( conditions );}

		result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release" ), QString::fromUtf8( "x64" ) ) );
		{auto conditions = getConditions(); conditions["arch"] = "x86_64"; conditions["config"] = "release";
		result.back().setConditions( conditions );}
	}

	return result;
}

bool GeneratorVc2013::getSlnDeploy() const
{
	return false;
}
