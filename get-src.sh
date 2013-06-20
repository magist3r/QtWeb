#!/bin/bash
TEMP_DIR='temp-src'
QT_URL='http://download.qt-project.org/official_releases/qt/4.8/4.8.4/'
QT_SRC='qt-everywhere-opensource-src-4.8.4.tar.gz'
QTWEBKIT_URL='http://gitorious.org/webkit/qtwebkit-23/archive-tarball/qtwebkit-2.3.1b'
QTWEBKIT_SRC='qtwebkit-2.3.1b.tar.gz'

if [ ! -e "$TEMP_DIR" ]; then
    mkdir "$TEMP_DIR"
fi

if [ ! -e "$TEMP_DIR/$QT_SRC" ]; then
    wget -O "$TEMP_DIR/$QT_SRC" "$QT_URL$QT_SRC"
fi

if [ ! -e "$TEMP_DIR/$QTWEBKIT_SRC" ]; then
    wget -O "$TEMP_DIR/$QTWEBKIT_SRC" "$QTWEBKIT_URL"
fi

tar -C src/qt --strip-components=1 -xf $TEMP_DIR/$QT_SRC
tar -C src/qt -xf $TEMP_DIR/$QTWEBKIT_SRC
