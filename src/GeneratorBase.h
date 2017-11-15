/*
 Copyright (c) 2015, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and
    the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
    the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "TinderBox.h"

#include <QDir>

// Expresses generator-specific variables used in selecting while files to copy
// from a Template. In addition to conditions, encodes
class GeneratorConditions {
  public:
	GeneratorConditions() {}
	GeneratorConditions( const QMap<QString,QString> &conditions )
		: mConditions( conditions )
	{}

	void	setCondition( const QString &param, const QString &value ) { mConditions[param] = value; }
	// true if conditions contain 'key' and value is either the same or wildcard
	bool	keyMatches( const QString &key, const QString &value ) const;
	bool	matches( const GeneratorConditions &rhs ) const;
	// return value of "config" condition or empty string if not present
	QString	getConfig() const;

  private:
	// map from condition to value, such as "compiler" -> "vc2015"
	QMap<QString,QString>   mConditions;
};

class GeneratorBase {
  public:
	GeneratorBase() {}
	virtual ~GeneratorBase() {}

	virtual std::vector<GeneratorConditions> getConditions() const = 0;
	virtual void    generate( class Instancer *master ) = 0;
};

typedef QSharedPointer<GeneratorBase>	GeneratorBaseRef;
