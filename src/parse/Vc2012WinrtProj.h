#pragma once

#include "VcProj.h"
#include <QSharedPointer>

typedef QSharedPointer<class Vc2012WinrtProj>		Vc2012WinrtProjRef;

class Vc2012WinrtProj : public VcProj {
  public:
	static Vc2012WinrtProjRef		createFromString( const QString &VcProj, const QString &VcProjFilters );

  private:
	Vc2012WinrtProj( const QString &vcprojString, const QString &vcProjFiltersString );

	virtual QString	getSlnHeaderString() const;
};
