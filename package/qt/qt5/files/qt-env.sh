#!/bin/sh

export QT_QPA_PLATFORM=eglfs:size=800x480
export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins
export QT_QPA_FONTDIR=/usr/lib/fonts
export QT_QPA_GENERIC_PLUGINS=tslib
export QT_QPA_GENERIC_PLUGINS=evdevmouse:/dev/input/event1
export QT_QPA_GENERIC_PLUGINS=evdevkeyboard:/dev/input/event2
