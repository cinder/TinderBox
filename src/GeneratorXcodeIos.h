#pragma once

#include "GeneratorXcodeBase.h"

class GeneratorXcodeIos : public GeneratorXcodeBase
{
  public:
	GeneratorXcodeIos();

	QMap<QString,QString>	getConditions() const;
	QString					getRootFolderName() const { return QString::fromUtf8( "xcode_ios" ); }

	virtual QList<QString>			getSdks() const { return mSdks; }
	
  protected:
	QList<QString>		mSdks;
};
