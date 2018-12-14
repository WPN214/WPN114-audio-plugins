TARGET = WPN114-vst
TEMPLATE = lib
CONFIG += c++11 dll
QT += quick widgets

localmod: DESTDIR = $$QML_MODULE_DESTDIR/WPN114/Time
else {
    DESTDIR = $$[QT_INSTALL_QML]/WPN114/Time
    QML_MODULE_DESTDIR = $$[QT_INSTALL_QML]
}

for(FILE,QMLDIR_FILES) {
    QMAKE_POST_LINK += $$quote(cp $${FILE} $${DESTDIR}$$escape_expand(\n\t))
}

WPN114_AUDIO_REPOSITORY = ../WPN114-audio
INCLUDEPATH += $$WPN114_AUDIO_REPOSITORY
LIBS += -L$$QML_MODULE_DESTDIR/WPN114/Audio -lWPN114-audio
QMAKE_LFLAGS += -Wl,-rpath,$$QML_MODULE_DESTDIR/WPN114/Audio

macx {
    LIBS += -framework CoreFoundation
}

HEADERS += $$PWD/external/vst/aeffect.h
HEADERS += $$PWD/external/vst/aeffectx.h
HEADERS += $$PWD/source/audioplugin.hpp
SOURCES += $$PWD/source/audioplugin.mm

SOURCES += $$PWD/qml_plugin.cpp
HEADERS += $$PWD/qml_plugin.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
