#pragma once

#include "VcProj.h"
#include <QSharedPointer>

typedef QSharedPointer<class Vc2013Proj>		Vc2013ProjRef;

class Vc2013Proj : public VcProj {
  public:
	static Vc2013ProjRef		createFromString( const QString &VcProj, const QString &VcProjFilters );

  private:
	Vc2013Proj( const QString &vcprojString, const QString &vcProjFiltersString );

	virtual QString	getSlnHeaderString() const;
};
