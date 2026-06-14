static:unix {
    CONFIG -= import_plugins
    QTPLUGIN += qxcb \
        qxcb-egl-integration \
        qxcb-glx-integration \
        qgif \
        qicns \
        qico \
        qjpeg \
        qtga \
        qtiff \
        qwbmp \
        qwebp \
        qgenericbearer
    SOURCES += $$PWD/staticplugins.cpp
}
