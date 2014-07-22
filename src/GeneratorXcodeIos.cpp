#include "GeneratorXcodeIos.h"

GeneratorXcodeIos::GeneratorXcodeIos()
	: GeneratorXcodeBase()
{
	mSdks.push_back( QString("device") );
	mSdks.push_back( QString("simulator") );
}

QMap<QString,QString> GeneratorXcodeIos::getConditions() const
{
    QMap<QString,QString> conditions;
    conditions["compiler"] = "xcode";
    conditions["os"] = "ios";
    conditions["sdk"] = "*";
    return conditions;
}
