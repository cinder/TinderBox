#pragma once

#include "VcProj.h"
#include <QSharedPointer>

typedef QSharedPointer<class Vc2015WinrtProj>		Vc2013WinrtProjRef;

class Vc2015WinrtProj : public VcProj {
  public:
	static Vc2013WinrtProjRef		createFromString( const QString &VcProj, const QString &VcProjFilters );

  private:
	Vc2015WinrtProj( const QString &vcprojString, const QString &vcProjFiltersString );

	virtual QString	getSlnHeaderString() const;
};
