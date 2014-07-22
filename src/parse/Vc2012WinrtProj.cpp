#include "Vc2012WinrtProj.h"

Vc2012WinrtProjRef	Vc2012WinrtProj::createFromString( const QString &VcProj, const QString &VcProjFilters )
{
	return Vc2012WinrtProjRef( new Vc2012WinrtProj( VcProj, VcProjFilters ) );
}

Vc2012WinrtProj::Vc2012WinrtProj( const QString &vcprojString, const QString &vcProjFiltersString )
	: VcProj( vcprojString, vcProjFiltersString )
{

}

QString	Vc2012WinrtProj::getSlnHeaderString() const
{
	QString result;

	result += "\r\n";
	result += "Microsoft Visual Studio Solution File, Format Version 12.00\r\n";
	result += "# Visual Studio Express 2012 for Windows 8\r\n";

	return result;
}
