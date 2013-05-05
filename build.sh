#!/usr/bin/env bash
SKIP_QT_BUILD=false
CLEAN_QT_BUILD=false
COMPILE_JOBS=4

PATCH0='0001-configure.patch'
PATCH1='0002-webkit-pro.patch'
PATCH2='0003-qtwebkit-pro.patch'
PATCH3='0004-qstyles-qrc.patch'
PATCH4='0005-qwidget-cpp.patch'
PATCH5='0006-webkit-disable-video-and-npapi.patch'
PATCH6='0021-linux-mkspec.patch'
PATCH7='0031-mac-qtbug-29373-00.patch'
#PATCH7='0031-mac-mkspec.patch'

OPTIONS=''
OPTIONS+=' -opensource'
OPTIONS+=' -confirm-license'
OPTIONS+=' -static'
OPTIONS+=' -release'
OPTIONS+=' -fast'

if [[ $OSTYPE = darwin* ]]; then
    OPTIONS+=' -platform unsupported/macx-clang'
    OPTIONS+=' -arch x86'
else
    OPTIONS+=' -system-freetype'
    OPTIONS+=' -fontconfig'
    OPTIONS+=' -reduce-relocations'
fi

OPTIONS+=' -webkit'
OPTIONS+=' -qt-libjpeg'
OPTIONS+=' -qt-libpng'
OPTIONS+=' -qt-zlib'
OPTIONS+=' -openssl'

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
OPTIONS+=' -no-script'
OPTIONS+=' -no-scripttools'
OPTIONS+=' -no-dbus'
OPTIONS+=' -no-stl'
OPTIONS+=' -no-libtiff'
OPTIONS+=' -no-libmng'
OPTIONS+=' -no-audio-backend'
OPTIONS+=' -no-phonon-backend'
OPTIONS+=' -no-gstreamer'
OPTIONS+=' -no-sql-sqlite'
OPTIONS+=' -no-accessibility'

until [ -z "$1" ]; do
    case $1 in
        "--skip-qt-build")
            SKIP_QT_BUILD=true
            shift;;
        "--clean-qt-build")
            CLEAN_QT_BUILD=true
            shift;;
        "--jobs")
            shift
            COMPILE_JOBS=$1
            shift;;
        "--help")
            echo "Usage: $0 [--jobs NUM]"
            echo
            echo "  --skip-qt-build             Skip build of Qt."
            echo "  --clean-qt-build            Clean Qt build tree."
            echo "  --jobs NUM                  How many parallel compile jobs to use. Defaults to 4."
            echo
            exit 0
            ;;
        *)
            echo "Unrecognised option: $1"
            exit 1;;
    esac
done

function qt_error {
    echo "Qt build error! See logs above."
    exit 1
}

#Applying patches for Qt
cd src/qt
if [ ! -e ./configure ]; then
    echo "Looks like you forgot to extract Qt sources in src/qt directory!"
    exit 1
fi
patch -p0 -N < "../qt-patches/$PATCH0"
patch -p0 -N < "../qt-patches/$PATCH1"
patch -p0 -N < "../qt-patches/$PATCH2"
patch -p0 -N < "../qt-patches/$PATCH3"
patch -p0 -N < "../qt-patches/$PATCH4"
patch -p0 -N < "../qt-patches/$PATCH5"
if [[ $OSTYPE = linux ]]; then
    patch -p0 -N < "../qt-patches/$PATCH6"
fi
if [[ $OSTYPE = darwin* ]]; then
    patch -p0 -N < "../qt-patches/$PATCH7"
fi

# make clean if we have previous build in src/qt
if $CLEAN_QT_BUILD; then
    make confclean
fi

if ! $SKIP_QT_BUILD; then
    ./configure -prefix $PWD $OPTIONS && make -j$COMPILE_JOBS || qt_error
fi
cd ../.. && rm -rf build && mkdir -p build && cd build
export QTDIR=../src/qt
../src/qt/bin/qmake -config release ../QtWeb.pro
make -j$COMPILE_JOBS

exit 0
