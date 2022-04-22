#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T10:30:29
#
#-------------------------------------------------

QT       += core gui xml multimedia multimediawidgets opengl network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DMHelper
TEMPLATE = app

install_it.path = $$PWD/../bin
install_it.files = $$PWD/binsrc/*

INSTALLS += \
    install_it

win32:RC_ICONS += dmhelper.ico
macx:ICON

SOURCES += main.cpp\
    dmh_vlc.cpp \
    dmhlogger.cpp \
    mainwindow.cpp \
    publishglframe.cpp \
    publishglmaprenderer.cpp \
    publishglmapvideorenderer.cpp \
    publishglrenderer.cpp \
    publishwindow.cpp \
    videoplayergl.cpp \
    videoplayerglplayer.cpp \
    videoplayerglvideo.cpp

HEADERS  += mainwindow.h \
    dmh_vlc.h \
    dmhlogger.h \
    publishglframe.h \
    publishglmaprenderer.h \
    publishglmapvideorenderer.h \
    publishglrenderer.h \
    publishwindow.h \
    dmconstants.h \
    videoplayergl.h \
    videoplayerglplayer.h \
    videoplayerglvideo.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc

OTHER_FILES +=

DISTFILES += \
    buildanddeploy_msvc32.cmd \
    preparebuilddirectory_msvc.cmd \
    resources/calendar.xml \
    resources/equipment.xml \
    resources/quickref_data.xml \
    resources/shops.xml \
    resources/tables/Indefinite Madness.xml \
    resources/tables/Long Term Madness.xml \
    resources/tables/Short Term Madness.xml

INCLUDEPATH += $$PWD/../../DMHelperShared/inc
DEPENDPATH += $$PWD/../../DMHelperShared/inc
DEPENDPATH += $$PWD/../../DMHelperShared/src

win32: LIBS += -L$$PWD/vlc -llibvlc
macx: LIBS += -F$$PWD/vlc/ -framework VLCKit

