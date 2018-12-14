QT += widgets

WPN114_AUDIO_REPOSITORY = ../WPN114-audio
INCLUDEPATH += $$WPN114_AUDIO_REPOSITORY
LIBS += -L$$[QT_INSTALL_QML]/WPN114/Audio -lWPN114-audio

macx {
    LIBS += -framework CoreFoundation
}

HEADERS += $$PWD/external/vst/aeffect.h
HEADERS += $$PWD/external/vst/aeffectx.h
HEADERS += $$PWD/source/audioplugin.hpp
SOURCES += $$PWD/source/audioplugin.mm

SOURCES += $$PWD/qml_plugin.cpp
HEADERS += $$PWD/qml_plugin.hpp
