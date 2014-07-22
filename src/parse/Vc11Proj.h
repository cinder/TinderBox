#pragma once

#include "VcProj.h"
#include <QSharedPointer>

typedef QSharedPointer<class Vc11Proj>		Vc11ProjRef;

class Vc11Proj : public VcProj {
  public:
	static Vc11ProjRef		createFromString( const QString &VcProj, const QString &VcProjFilters );

  private:
	Vc11Proj( const QString &vcprojString, const QString &vcProjFiltersString );

	virtual QString	getSlnHeaderString() const;
};
