QT  += widgets winextras network

CONFIG += c++11

TARGET = modmngr

RC_FILE = exeicon.qrc
#QMAKE_CXXFLAGS += -Werror

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    sortfilterproxymodel.cpp \
    archiver.cpp \
    wizardpage.cpp \
    nexusinterface.cpp \
    pluginmanager.cpp \
    modmanager.cpp \
    bashimportwizardpage.cpp

HEADERS += \
    mainwindow.h \
    sortfilterproxymodel.h \
    archiver.h \
    wizardpage.h \
    modInfo.h \
    nexusinterface.h \
    pluginmanager.h \
    modmanager.h \
    bashimportwizardpage.h

FORMS += \
    mainwindow.ui \
    wizardpage.ui \
    bashimportwizardpage.ui

RESOURCES += \
    resources.qrc \

OTHER_FILES += \
    Original.ico
