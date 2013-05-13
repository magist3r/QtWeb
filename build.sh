#!/bin/bash
PATCHDIR="$(pwd)/src/qt-patches"
SKIP_QT_BUILD=false
CLEAN_QT_BUILD=false
CROSS_COMPILE=false
CROSS_COMPILE_PREFIX='i686-w64-mingw32-'
COMPILE_JOBS=8

PATCH0='0001-configure.patch'
PATCH1='0002-webkit-pro.patch'
PATCH2='0003-qtwebkit-pro.patch'
PATCH3='0004-qstyles-qrc.patch'
PATCH4='0005-qwidget-cpp.patch'
PATCH5='0006-webkit-disable-video-and-npapi.patch'

#linux
PATCHL0='0021-linux-mkspec.patch'
PATCHL1='0022-linux-qgtkstyle-qtbug-23569.patch'

#mac
PATCHM0='0031-mac-qtbug-29373-00.patch'

#win32
PATCHW0='0012-windows-webcore-pro.patch'
PATCHW1='0013-windows-dotnet-style.patch'
PATCHW2='0014-windows-mkspec-cross-compile.patch'

until [ -z "$1" ]; do
    case $1 in
        "--skip-qt-build")
            SKIP_QT_BUILD=true
            shift;;
        "--clean-qt-build")
            CLEAN_QT_BUILD=true
            shift;;
        "--cross-compile")
            CROSS_COMPILE=true
            shift
            if [ ! -z "$1" ]; then
                CROSS_COMPILE_PREFIX="$1"
                shift
            fi
            ;;
        "--jobs")
            shift
            COMPILE_JOBS=$1
            shift;;
        "--help")
            echo "Usage: $0 [--jobs NUM]"
            echo
            echo "  --skip-qt-build             Skip build of Qt."
            echo "  --clean-qt-build            Clean Qt build tree."
            echo "  --cross-compile [prefix]    Cross-compile Qt for windows on linux (requires static openssl installed)"
            echo "                              and specify prefix (default: i686-w64-mingw32-)"
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

if ! $SKIP_QT_BUILD; then

    OPTIONS=''
    OPTIONS+=' -opensource'
    OPTIONS+=' -confirm-license'
    OPTIONS+=' -static'
    OPTIONS+=' -release'
    OPTIONS+=' -v'
    OPTIONS+=' -fast'

    if [[ $OSTYPE = darwin* ]]; then
        OPTIONS+=' -arch x86'
        OPTIONS+=' -carbon' # use carbon for compatibility reasons
        OPTIONS+=' -openssl'
    else
        if $CROSS_COMPILE; then
            OPTIONS+=' -xplatform win32-g++'
            OPTIONS+=" -device-option CROSS_COMPILE=$CROSS_COMPILE_PREFIX"
            OPTIONS+=' -openssl-linked'
        else
            OPTIONS+=' -system-freetype'
            OPTIONS+=' -fontconfig'
            OPTIONS+=' -reduce-relocations'
            OPTIONS+=' -openssl'
        fi
    fi

    OPTIONS+=' -webkit'
    OPTIONS+=' -qt-libjpeg'
    OPTIONS+=' -qt-libpng'
    OPTIONS+=' -qt-zlib'

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
    OPTIONS+=' -D QT_NO_STYLE_CDE'
    OPTIONS+=' -D QT_NO_STYLE_MOTIF'
    OPTIONS+=' -D QT_NO_STYLE_PLASTIQUE'

    #Applying patches for Qt
    cd src/qt
    if [ ! -e ./configure ]; then
        echo "Looks like you forgot to extract Qt sources in src/qt directory!"
        exit 1
    fi

    patch -p0 -N < "$PATCHDIR/$PATCH0"
    patch -p0 -N < "$PATCHDIR/$PATCH1"
    patch -p0 -N < "$PATCHDIR/$PATCH2"
    patch -p0 -N < "$PATCHDIR/$PATCH3"
    patch -p0 -N < "$PATCHDIR/$PATCH4"
    patch -p0 -N < "$PATCHDIR/$PATCH5"

    if [[ $OSTYPE = linux* ]] && ! $CROSS_COMPILE; then
        patch -p0 -N < "$PATCHDIR/$PATCHL0"
        patch -p0 -N < "$PATCHDIR/$PATCHL1"
    fi

    if [[ $OSTYPE = darwin* ]]; then
        patch -p1 -N < "$PATCHDIR/$PATCHM0"
    fi

    if $CROSS_COMPILE; then
        patch -p0 -N < "$PATCHDIR/$PATCHW0"
        patch -p0 -N < "$PATCHDIR/$PATCHW1"
        patch -p0 -N < "$PATCHDIR/$PATCHW2"
    fi

    # make clean if we have previous build in src/qt
    if $CLEAN_QT_BUILD; then
        make confclean
    fi

    ./configure -prefix $PWD $OPTIONS && make -j$COMPILE_JOBS || qt_error
    cd ../..
fi # end of Qt build

rm -rf build && mkdir -p build && cd build
export QTDIR=../src/qt
../src/qt/bin/qmake -config release ../QtWeb.pro
make -j$COMPILE_JOBS

#fix qt_menu.nib issue
if [[ $OSTYPE = darwin* ]]; then
    cd ..
    cp -r src/qt/src/gui/mac/qt_menu.nib build/release/QtWeb.app/Contents/Resources/
fi

exit 0
