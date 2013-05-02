#!/bin/sh
OPTIONS=''
OPTIONS+=' -opensource'
OPTIONS+=' -confirm-license'
OPTIONS+=' -v'
OPTIONS+=' -static'

#linux
if [ "$OSTYPE" = "linux" ]; then
OPTIONS+=' -system-freetype'
OPTIONS+=' -fontconfig'
fi

OPTIONS+=' -release'
OPTIONS+=' -fast'
OPTIONS+=' -nomake demos'
OPTIONS+=' -nomake docs'
OPTIONS+=' -nomake examples'
OPTIONS+=' -nomake translations'
OPTIONS+=' -nomake tests'
OPTIONS+=' -nomake tools'
OPTIONS+=' -no-declarative'
OPTIONS+=' -no-multimedia'
OPTIONS+=' -no-opengl'
OPTIONS+=' -no-openvg'
OPTIONS+=' -no-phonon'
OPTIONS+=' -no-qt3support'
#OPTIONS+=' -no-script'
OPTIONS+=' -no-scripttools'
#OPTIONS+=' -no-svg'
OPTIONS+=' -qt-libjpeg'
OPTIONS+=' -qt-libpng'
OPTIONS+=' -qt-zlib'
OPTIONS+=' -openssl'
OPTIONS+=' -no-dbus'
OPTIONS+=' -no-stl'
OPTIONS+=' -no-libtiff'
OPTIONS+=' -no-libmng'

./configure $OPTIONS

exit 0
