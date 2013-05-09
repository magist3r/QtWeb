#!/usr/bin/env bash
URL='http://download.qt-project.org/official_releases/qt/4.8/4.8.4/'
QTSRC='qt-everywhere-opensource-src-4.8.4.tar.gz'
if [ ! -e "src/qt/$QTSRC" ]; then
    wget -O "src/qt/$QTSRC" "$URL$QTSRC"
fi
tar -C src/qt --strip-components=1 -xf src/qt/$QTSRC
