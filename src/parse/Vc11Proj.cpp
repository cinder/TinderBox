#include "Vc11Proj.h"

Vc11ProjRef	Vc11Proj::createFromString( const QString &VcProj, const QString &VcProjFilters )
{
	return Vc11ProjRef( new Vc11Proj( VcProj, VcProjFilters ) );
}

Vc11Proj::Vc11Proj( const QString &vcprojString, const QString &vcProjFiltersString )
	: VcProj( vcprojString, vcProjFiltersString )
{

}

QString	Vc11Proj::getSlnHeaderString() const
{
	QString result;

	result += "\r\n";
	result += "Microsoft Visual Studio Solution File, Format Version 12.00\r\n";
	result += "# Visual Studio Express 2012 for Windows Desktop\r\n";

	return result;
}
