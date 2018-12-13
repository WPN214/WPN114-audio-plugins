#include "qml_plugin.hpp"

#include <source/audioplugin.hpp>
#include <QQmlEngine>
#include <qqml.h>

void qml_plugin::registerTypes(const char *uri)
{
    Q_UNUSED    ( uri );

    qmlRegisterType<AudioPlugin, 1> ( "WPN114.Vst", 1, 0, "AudioPlugin" );
}
