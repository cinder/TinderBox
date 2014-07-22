#include "GeneratorVc11.h"
#include "ProjectTemplateManager.h"
#include "Util.h"
#include "Vc11Proj.h"

#include <fstream>

QMap<QString,QString> GeneratorVc11::getConditions() const
{
	QMap<QString,QString> conditions;
	conditions["compiler"] = "vc2012";
	conditions["os"] = "msw";
	return conditions;
}

QString GeneratorVc11::getCompiler() const
{
	return QString( "110_xp" );
}

VcProjRef GeneratorVc11::createVcProj( const QString &vcProj, const QString &vcProjFilters )
{
	return Vc11Proj::createFromString( vcProj, vcProjFilters );
}

std::vector<QString> GeneratorVc11::getArchitectures() const
{
	std::vector<QString> result;
	result.push_back( QString::fromUtf8( "Win32" ) );
	return result;
}

bool GeneratorVc11::getSlnDeploy() const
{
	return false;
}
