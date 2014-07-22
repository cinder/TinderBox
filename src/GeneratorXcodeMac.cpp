#include "GeneratorXcodeMac.h"

GeneratorXcodeMac::GeneratorXcodeMac()
	: GeneratorXcodeBase()
{
	mSdks.push_back( QString() );
}

QMap<QString,QString> GeneratorXcodeMac::getConditions() const
{
    QMap<QString,QString> conditions;
    conditions["compiler"] = "xcode";
    conditions["os"] = "macosx";
    return conditions;
}
