#include <QtWidgets>
#include <QFontDatabase>
#include "MainWizard.h"

#include <iostream>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	QCoreApplication::setOrganizationName( "libcinder" );
	QCoreApplication::setOrganizationDomain( "libcinder.org" );
	QCoreApplication::setApplicationName( "TinderBox" );

#if defined Q_OS_MACX
	if ( QSysInfo::MacintoshVersion > (QSysInfo::MV_10_7 + 1) ) {
		// fix Mac OS X 10.9 (mavericks) font issue
		// https://bugreports.qt-project.org/browse/QTBUG-32789
		QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
	}
#endif

	MainWizard wizard;

#if defined( Q_OS_WIN32 )
	QIcon icon( ":/tinderbox_app_icon.ico" );
	wizard.setWindowIcon( icon );
#endif

	wizard.show();
	
	return a.exec();
}
