#-------------------------------------------------
#
# Project created by QtCreator 2014-07-19T23:08:43
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TinderBox
TEMPLATE = app

INCLUDEPATH += src/ src/parse/

macx {
    ICON = icons/TinderBox.icns
}

win32 {
    CONFIG += C++11
    RC_FILE = Resources_Win.rc
}

SOURCES += \
    src/CinderBlock.cpp \
    src/CinderBlockManager.cpp \
    src/ErrorList.cpp \
    src/FirstTimeDlg.cpp \
    src/GeneratorVc2012Winrt.cpp \
    src/GeneratorVc2013.cpp \
    src/GeneratorVcBase.cpp \
    src/GeneratorXcodeBase.cpp \
    src/GeneratorXcodeIos.cpp \
    src/GeneratorXcodeMac.cpp \
    src/Instancer.cpp \
    src/main.cpp \
    src/MainWizard.cpp \
    src/Preferences.cpp \
    src/ProjectTemplate.cpp \
    src/ProjectTemplateManager.cpp \
    src/Template.cpp \
    src/Util.cpp \
    src/WizardPageCinderBlocks.cpp \
    src/WizardPageMain.cpp \
    src/parse/PList.cpp \
    src/parse/Vc2012WinrtProj.cpp \
    src/parse/Vc2013Proj.cpp \
    src/parse/VcProj.cpp \
    src/parse/XCodeProj.cpp \
    src/pugixml/pugixml.cpp \
    src/WizardPageEnvOptions.cpp

HEADERS  += \
    src/CinderBlock.h \
    src/CinderBlockManager.h \
    src/ErrorList.h \
    src/FirstTimeDlg.h \
    src/GeneratorBase.h \
    src/GeneratorVc2012Winrt.h \
    src/GeneratorVc2013.h \
    src/GeneratorVcBase.h \
    src/GeneratorXcodeBase.h \
    src/GeneratorXcodeIos.h \
    src/GeneratorXcodeMac.h \
    src/Instancer.h \
    src/MainWizard.h \
    src/Preferences.h \
    src/ProjectTemplate.h \
    src/ProjectTemplateManager.h \
    src/Template.h \
    src/TinderBox.h \
    src/Util.h \
    src/WizardPageCinderBlocks.h \
    src/WizardPageMain.h \
    src/parse/PList.h \
    src/parse/Vc2012WinrtProj.h \
    src/parse/Vc2013Proj.h \
    src/parse/VcProj.h \
    src/parse/XCodeProj.h \
    src/pugixml/pugiconfig.hpp \
    src/pugixml/pugixml.hpp \
    src/WizardPageEnvOptions.h

RESOURCES += \
    TinderBox.qrc

FORMS += \
    ui/ErrorListDialog.ui \
    ui/FirstTimeDlg.ui \
    ui/PrefsDlg.ui \
    ui/WizardPageCinderBlocks.ui \
    ui/WizardPageMain.ui \
	ui/WizardPageEnvOptions.ui
