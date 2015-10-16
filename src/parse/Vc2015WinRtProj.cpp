#include "Vc2015WinrtProj.h"

Vc2013WinrtProjRef Vc2015WinrtProj::createFromString( const QString &VcProj, const QString &VcProjFilters )
{
    return Vc2013WinrtProjRef( new Vc2015WinrtProj( VcProj, VcProjFilters ) );
}

Vc2015WinrtProj::Vc2015WinrtProj( const QString &vcprojString, const QString &vcProjFiltersString )
	: VcProj( vcprojString, vcProjFiltersString )
{

}

QString	Vc2015WinrtProj::getSlnHeaderString() const
{
	QString result;

	result += "\r\n";
	result += "Microsoft Visual Studio Solution File, Format Version 12.00\r\n";
	result += "# Visual Studio 2013\r\n";

	return result;
}
