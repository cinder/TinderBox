#include "GeneratorBase.h"

QString	GeneratorConditions::getConfig() const
{
	return mConditions.contains( "config" ) ? mConditions["config"] : QString();
}

bool GeneratorConditions::keyMatches( const QString &key, const QString &value ) const
{
	return mConditions.contains( key ) && ( mConditions[key] == value || mConditions[key] == "*" );
}

bool GeneratorConditions::matches( const GeneratorConditions &rhs ) const
{
	for( auto conditionIt = rhs.mConditions.begin(); conditionIt != rhs.mConditions.end(); ++conditionIt )
		if( ! keyMatches( conditionIt.key(), conditionIt.value() ) )
			return false;
	return true;
}

