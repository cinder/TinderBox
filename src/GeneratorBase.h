#pragma once

#include "TinderBox.h"
#include "ProjectTemplate.h"

#include <QDir>

class GeneratorBase {
  public:
	GeneratorBase() {}
	virtual ~GeneratorBase() {}

    virtual QMap<QString,QString>   getConditions() const = 0;
	virtual void    generate( class Instancer *master ) = 0;
};

typedef QSharedPointer<GeneratorBase>	GeneratorBaseRef;
