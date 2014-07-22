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

VcProjRef GeneratorVc2013::createVcProj( const QString &vcProj, const QString &vcProjFilters )
{
	return Vc2013Proj::createFromString( vcProj, vcProjFilters );
}

std::vector<QString> GeneratorVc2013::getArchitectures() const
{
	std::vector<QString> result;
	result.push_back( QString::fromUtf8( "Win32" ) );
	return result;
}

bool GeneratorVc2013::getSlnDeploy() const
{
	return false;
}
