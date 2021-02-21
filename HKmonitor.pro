#-------------------------------------------------
#
# Project created by QtCreator 2021-02-20T15:10:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent

TARGET = HKmonitor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    commoninterface.cpp \
    ddcdoc.cpp \
    globaldef.cpp \
    monitorwidget.cpp \
    commoninterface.cpp \
    monitor/cameralist.cpp \
    monitor/monitorpreviewwidget.cpp \
    monitor/playbackwidget.cpp \
    monitor/previewitem.cpp \
    monitor/previewwidget.cpp \
    monitor/systemconfigurewidget.cpp \
    monitor/timebar.cpp

HEADERS += \
        mainwindow.h \
    commoninterface.h \
    ddcdoc.h \
    globaldef.h \
    monitorwidget.h \
    monitor/cameralist.h \
    monitor/monitorpreviewwidget.h \
    monitor/playbackwidget.h \
    monitor/previewitem.h \
    monitor/previewwidget.h \
    monitor/systemconfigurewidget.h \
    monitor/timebar.h

FORMS += \
        mainwindow.ui \
    monitor/monitorpreviewwidget.ui \
    monitor/playbackwidget.ui

LIBS += -L$$PWD/HKLibs/ -lHCCore -lHCNetSDK -lPlayCtrl -lGdiPlus
LIBS += -L$$PWD/HKLibs/HCNetSDKCom/ -lHCAlarm -lHCGeneralCfgMgr -lHCPreview

RESOURCES += \
    resource.qrc
