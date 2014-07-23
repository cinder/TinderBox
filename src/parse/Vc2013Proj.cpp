#include "Vc2013Proj.h"

Vc2013ProjRef Vc2013Proj::createFromString( const QString &VcProj, const QString &VcProjFilters )
{
	return Vc2013ProjRef( new Vc2013Proj( VcProj, VcProjFilters ) );
}

Vc2013Proj::Vc2013Proj( const QString &vcprojString, const QString &vcProjFiltersString )
	: VcProj( vcprojString, vcProjFiltersString )
{

}

QString	Vc2013Proj::getSlnHeaderString() const
{
	QString result;

	result += "\r\n";
	result += "Microsoft Visual Studio Solution File, Format Version 12.00\r\n";
    result += "# Visual Studio 2013\r\n";

	return result;
}
