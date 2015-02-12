#include "GeneratorVc2012Winrt.h"
#include "ProjectTemplateManager.h"
#include "Util.h"
#include "Vc2012WinrtProj.h"

#include <fstream>

QMap<QString,QString> GeneratorVc2012Winrt::getConditions() const
{
	QMap<QString,QString> conditions;
	conditions["compiler"] = "vc2012";
	conditions["os"] = "winrt";
	return conditions;
}

QString GeneratorVc2012Winrt::getCompiler() const
{
	return QString( "110" );
}

VcProjRef GeneratorVc2012Winrt::createVcProj( const QString &VcProj, const QString &VcProjFilters )
{
    return Vc2012WinrtProj::createFromString( VcProj, VcProjFilters );
}

std::vector<VcProj::ProjectConfiguration> GeneratorVc2012Winrt::getPlatformConfigurations() const
{
    std::vector<VcProj::ProjectConfiguration> result;
    result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug" ), QString::fromUtf8( "ARM" ) ) );
    result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release" ), QString::fromUtf8( "ARM" ) ) );
    result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug" ), QString::fromUtf8( "Win32" ) ) );
    result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release" ), QString::fromUtf8( "Win32" ) ) );
    result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Debug" ), QString::fromUtf8( "x64" ) ) );
    result.push_back( VcProj::ProjectConfiguration( QString::fromUtf8( "Release" ), QString::fromUtf8( "x64" ) ) );

    return result;
}

bool GeneratorVc2012Winrt::getSlnDeploy() const
{
	return true;
}
