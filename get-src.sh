#!/bin/bash
TEMP_DIR='temp-src'
QT_URL='http://download.qt-project.org/official_releases/qt/4.8/4.8.4/'
QT_SRC='qt-everywhere-opensource-src-4.8.4.tar.gz'
QTWEBKIT_URL='git://gitorious.org/+qtwebkit-developers/webkit/qtwebkit-23.git'
QTWEBKIT_SRC='src/qtwebkit-23'

if [ ! -e "$TEMP_DIR" ]; then
    mkdir "$TEMP_DIR"
fi

if [ ! -e "$TEMP_DIR/$QT_SRC" ]; then
    wget -O "$TEMP_DIR/$QT_SRC" "$QT_URL$QT_SRC"
fi

tar -C src/qt --strip-components=1 -xf $TEMP_DIR/$QT_SRC

if [ ! -e "$QTWEBKIT_SRC" ]; then
    git clone --depth 1 "$QTWEBKIT_URL" "$QTWEBKIT_SRC"
fi

