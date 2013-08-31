#!/bin/bash
TEMP_DIR='temp-src'
QT_URL='http://download.qt-project.org/official_releases/qt/4.8/4.8.5/'
QT_SRC='qt-everywhere-opensource-src-4.8.5.tar.gz'
QTWEBKIT_URL='http://gitorious.org/webkit/qtwebkit-23/archive-tarball/qtwebkit-2.3.1b'
QTWEBKIT_SRC='qtwebkit-2.3.1b.tar.gz'
OPENSSL_URL='http://www.openssl.org/source/'
OPENSSL_SRC='openssl-1.0.1e.tar.gz'

if [ ! -e "$TEMP_DIR" ]; then
    mkdir "$TEMP_DIR"
fi

if [ ! -e "$TEMP_DIR/$QT_SRC" ]; then
    wget -O "$TEMP_DIR/$QT_SRC" "$QT_URL$QT_SRC"
fi

if [ ! -e "$TEMP_DIR/$QTWEBKIT_SRC" ]; then
    wget -O "$TEMP_DIR/$QTWEBKIT_SRC" "$QTWEBKIT_URL"
fi

if [ ! -e "$TEMP_DIR/$OPENSSL_SRC" ] && [[ $OSTYPE = cygwin ]]; then
    wget -O "$TEMP_DIR/$OPENSSL_SRC" "$OPENSSL_URL$OPENSSL_SRC"
fi

if [ ! -e src/qt/configure ]; then tar -C src/qt --strip-components=1 -xf $TEMP_DIR/$QT_SRC; fi
if [ ! -e src/qt/webkit-qtwebkit-23 ]; then tar -C src/qt -xf $TEMP_DIR/$QTWEBKIT_SRC; fi
if [[ $OSTYPE = cygwin ]] && [ ! -e src/qt/openssl-1.0.1e ]; then tar -C src/qt -xf $TEMP_DIR/$OPENSSL_SRC; fi
