#include "GeneratorBase.h"

QString	GeneratorConditions::getConfig() const
{
	return mConditions.contains( "config" ) ? mConditions["config"] : QString();
}

bool GeneratorConditions::matches( const GeneratorConditions &rhs ) const
{
	for( auto conditionIt = rhs.mConditions.begin(); conditionIt != rhs.mConditions.end(); ++conditionIt )
		if( mConditions.contains( conditionIt.key() ) && mConditions[conditionIt.key()] != conditionIt.value() )
			return false;
	return true;
}

