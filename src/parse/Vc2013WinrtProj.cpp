#include "Vc2013WinrtProj.h"

Vc2013WinrtProjRef Vc2013WinrtProj::createFromString( const QString &VcProj, const QString &VcProjFilters )
{
	return Vc2013WinrtProjRef( new Vc2013WinrtProj( VcProj, VcProjFilters ) );
}

Vc2013WinrtProj::Vc2013WinrtProj( const QString &vcprojString, const QString &vcProjFiltersString )
	: VcProj( vcprojString, vcProjFiltersString )
{

}

QString	Vc2013WinrtProj::getSlnHeaderString() const
{
	QString result;

	result += "\r\n";
	result += "Microsoft Visual Studio Solution File, Format Version 12.00\r\n";
	result += "# Visual Studio 2013\r\n";

	return result;
}
