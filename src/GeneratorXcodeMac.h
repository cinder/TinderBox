#pragma once

#include "GeneratorXcodeBase.h"

class GeneratorXcodeMac : public GeneratorXcodeBase
{
  public:
	GeneratorXcodeMac();

	QMap<QString,QString>	getConditions() const;
	QString					getRootFolderName() const { return QString::fromUtf8( "xcode" ); }
	
	virtual QList<QString>			getSdks() const { return mSdks; }
	
  protected:
	QList<QString>		mSdks;
};
