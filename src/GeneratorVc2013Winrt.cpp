#include "GeneratorVc2013Winrt.h"
#include "ProjectTemplateManager.h"
#include "Util.h"
#include "Vc2013WinrtProj.h"

#include <fstream>

QMap<QString,QString> GeneratorVc2013Winrt::getConditions() const
{
	QMap<QString,QString> conditions;
	conditions["compiler"] = "vc2013";
	conditions["os"] = "winrt";
	return conditions;
}

QString GeneratorVc2013Winrt::getCompiler() const
{
	return QString( "120" );
}

VcProjRef GeneratorVc2013Winrt::createVcProj( const QString &VcProj, const QString &VcProjFilters )
{
	return Vc2013WinrtProj::createFromString( VcProj, VcProjFilters );
}

std::vector<VcProj::ProjectConfiguration> GeneratorVc2013Winrt::getPlatformConfigurations() const
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

bool GeneratorVc2013Winrt::getSlnDeploy() const
{
	return true;
}
