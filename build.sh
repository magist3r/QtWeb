#!/bin/bash
PATCHDIR="$(pwd)/qt-patches"
USE_QTWEBKIT_23=false
SSL_LIBS=''
export QTDIR=$(pwd)/src/qt
export PATH=$QTDIR/bin:$PATH
SKIP_QT_BUILD=false
CLEAN_QT_BUILD=false
CROSS_COMPILE=false
CROSS_COMPILE_PREFIX='i686-w64-mingw32-'
COMPILE_JOBS=8
MAKE_COMMAND="make -j$COMPILE_JOBS"
CONFIGURE_COMMAND="./configure"
QT_PATCHES=() # Array of qt patches
QTWEBKIT_PATCHES=() # Array of qtwebkit patches
MKSPEC_PATCHES=() # Patches for qmake.conf 

if [[ $OSTYPE = cygwin ]]; then
    #setup msvc build environment
    export OPENSSL="$QTDIR/openssl"
    export WindowsSdkDir=$(cygpath -ua "C:/Program Files/Microsoft SDKs/Windows/v7.1")
    export VSINSTALLDIR=$(cygpath -ua "C:/Program Files (x86)/Microsoft Visual Studio 10.0/")
    export VCINSTALLDIR=$(cygpath -ua "C:/Program Files (x86)/Microsoft Visual Studio 10.0/VC")
    export DevEnvDir="$VSINSTALLDIR/Common7/IDE"
    export FrameworkVersion='v4.0.30319'
    export Framework35Version='v3.5'
    export FrameworkDir=$(cygpath -ua "$SYSTEMROOT/Microsoft.NET/Framework")
    export TARGET_CPU='x86'
    export QTDIR=$(cygpath -wlpa "$QTDIR")
    MAKE_COMMAND=nmake

    export LIBPATH="$FrameworkDir/$FrameworkVersion:$FrameworkDir/$Framework35Version:$VCINSTALLDIR/lib:$LIBPATH"
    export PATH="$LIBPATH:$VCINSTALLDIR/bin:$VSINSTALLDIR/Common7/Tools:$VSINSTALLDIR/Common7/IDE:$VCINSTALLDIR/VCPackages:$WindowsSdkDir/Bin:$OPENSSL/bin:$QTDIR/bin:$PATH"
    export LIB=$(cygpath -wlpa "$VCINSTALLDIR/lib:$WindowsSdkDir/Lib:$OPENSSL/lib:$LIB")
    export INCLUDE=$(cygpath -wlpa "$VCINSTALLDIR/include/:$WindowsSdkDir/Include/:$OPENSSL/include/:$INCLUDE")
fi

until [ -z "$1" ]; do
    case $1 in
        "--skip-qt-build")
            SKIP_QT_BUILD=true
            shift;;
        "--clean-qt-build")
            CLEAN_QT_BUILD=true
            shift;;
        "--use-qtwebkit-23")
            USE_QTWEBKIT_23=true
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
            echo "  --use-qtwebkit-23           Build QtWeb with QtWebKit 2.3."
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

function qtwebkit_error {
    echo "QtWebKit 2.3 build error! See logs above."
    exit 1
}

QT_PATCHES+=('0004-qstyles-qrc.patch')
QT_PATCHES+=('0005-qwidget-cpp.patch')

if ! $USE_QTWEBKIT_23; then
    QTWEBKIT_PATCHES+=('0001-configure.patch')
    QTWEBKIT_PATCHES+=('0002-webkit-pro.patch')
    QTWEBKIT_PATCHES+=('0003-qtwebkit-pro.patch')
    QTWEBKIT_PATCHES+=('0006-webkit-disable-video.patch')
else
    QTWEBKIT_PATCHES+=('0041-qtwebkit-23-dont-link-with-jpeg-and-png.patch')
    QTWEBKIT_PATCHES+=('0042-qtwebkit-23-dont-link-with-sqlite.patch')
fi

if ! $SKIP_QT_BUILD; then

    OPTIONS=''
    OPTIONS+=' -opensource'
    OPTIONS+=' -confirm-license'
    OPTIONS+=' -static'
    OPTIONS+=' -release'
    OPTIONS+=' -silent'
    OPTIONS+=' -fast'

    OPTIONS+=' -openssl'
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
    OPTIONS+=' -no-sql-sqlite'
    OPTIONS+=' -no-accessibility'
    OPTIONS+=' -D QT_NO_STYLE_CDE'
    OPTIONS+=' -D QT_NO_STYLE_MOTIF'
    OPTIONS+=' -optimized-qmake'

    case $OSTYPE in
    darwin*)
        OPTIONS+=' -arch x86'
        OPTIONS+=' -carbon' # use carbon for compatibility reasons
        OPTIONS+=' -sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk'
        
        if ! $USE_QTWEBKIT_23; then QTWEBKIT_PATCHES+=('0031-mac-fix-includepath-for-npapi.patch'); fi
        ;;
    cygwin)
        SSL_LIBS="OPENSSL_LIBS=\"ssleay32.lib libeay32.lib crypt32.lib gdi32.lib\""
        CONFIGURE_COMMAND="./configure.exe"
        OPTIONS=("${OPTIONS[@]// -silent}")
        OPTIONS=("${OPTIONS[@]// -openssl}")
        OPTIONS=("${OPTIONS[@]// -optimized-qmake}")

        OPTIONS+=' -openssl-linked'
        OPTIONS+=' -mp'
        OPTIONS+=' -no-s60'

        MKSPEC_PATCHES+=('0011-windows-mkspec.patch')
        QT_PATCHES+=('0013-windows-dotnet-style.patch')
        if ! $USE_QTWEBKIT_23; then
            QTWEBKIT_PATCHES+=('0012-windows-webcore-pro.patch')
        else
            QTWEBKIT_PATCHES+=('0015-windows-qtwebkit-23-cygwin.patch')
        fi
        ;;
    beos)
        OPTIONS+=' -no-largefile'
        OPTIONS+=' -no-pch'
        ;;
    linux*)
        if $CROSS_COMPILE; then
            SSL_LIBS='-lssl -lcrypto -lcrypt32 -lgdi32'
            OPTIONS=("${OPTIONS[@]// -openssl}")
            OPTIONS+=' -openssl-linked'
            OPTIONS+=' -xplatform win32-g++'
            OPTIONS+=" -device-option CROSS_COMPILE=$CROSS_COMPILE_PREFIX"
            
            QT_PATCHES+=('0013-windows-dotnet-style.patch')
            MKSPEC_PATCHES+=('0014-windows-mkspec-cross-compile.patch')
            if ! $USE_QTWEBKIT_23; then QTWEBKIT_PATCHES+=('0012-windows-webcore-pro.patch'); fi
            
        else
            OPTIONS+=' -no-gstreamer'
            OPTIONS+=' -system-freetype'
            OPTIONS+=' -fontconfig'
            OPTIONS+=' -glib'
            OPTIONS+=' -gtkstyle'
            OPTIONS+=' -reduce-relocations'
            OPTIONS+=' -platform linux-g++-32'
            OPTIONS+=' -D OLD_GLIBC'
            OPTIONS+=' -D OLD_GLIB'
            OPTIONS+=' -D _GNU_SOURCE'

            MKSPEC_PATCHES+=('0021-linux-mkspec.patch')

            QT_PATCHES+=('0022-linux-qgtkstyle-qtbug-23569.patch')
            QT_PATCHES+=('0023-linux-link-with-old-glibc.patch')
            QT_PATCHES+=('0025-linux-link-with-old-glib.patch')
            
            if ! $USE_QTWEBKIT_23; then QTWEBKIT_PATCHES+=('0024-linux-webkit-not-link-with-gio.patch'); fi
        fi
        ;;
    *)
        echo "Your platform $OSTYPE is unsupported."
        exit 1
        ;;
    esac
    
    if $USE_QTWEBKIT_23; then
        OPTIONS+=' -no-webkit'
    else
        OPTIONS+=' -webkit'
    fi

    pushd src/qt
    if [ ! -e ./configure ]; then
        echo "Looks like you forgot to extract Qt sources in src/qt directory!"
        exit 1
    fi
    
    if $USE_QTWEBKIT_23 && [ ! -e ./webkit-qtwebkit-23 ]; then
        echo "Looks like you forgot to extract QtWebKit 2.3 sources in src/qt/webkit-qtwebkit-23 directory!"
        exit 1
    fi
    
    # make clean if we have previous build in src/qt
    if $CLEAN_QT_BUILD; then
        $MAKE_COMMAND confclean
        if $USE_QTWEBKIT_23; then 
            rm -rf webkit-qtwebkit-23/WebKitBuild 
        fi
    fi
    
    #Applying patches for Qt and QtWebKit
    for i in "${QT_PATCHES[@]}"; do
        patch -p0 -N < "$PATCHDIR/$i"
    done
    
    for j in "${MKSPEC_PATCHES[@]}"; do
        patch -p0 -N < "$PATCHDIR/$j"
    done
    
    for k in "${QTWEBKIT_PATCHES[@]}"; do
        patch -p0 -N < "$PATCHDIR/$k"
    done

    if [[ $OSTYPE = cygwin ]]; then
        #build openssl
        pushd $QTDIR/openssl-1.0.1e/
        perl Configure VC-WIN32 no-asm --prefix=$QTDIR/openssl
        ms/do_ms.bat
        $MAKE_COMMAND -f ms/nt.mak install
        popd
    fi

    COMMAND="$CONFIGURE_COMMAND -prefix "$QTDIR" $OPTIONS "$SSL_LIBS""
    COMMAND=$(echo $COMMAND | sed 's/ *$//g')
    $COMMAND || qt_error
    $MAKE_COMMAND || qt_error
    
    #qtwebkit build
    if $USE_QTWEBKIT_23; then
        pushd webkit-qtwebkit-23
        QTWEBKIT_OPTIONS=''
        QTWEBKIT_OPTIONS+=' --qt'
        QTWEBKIT_OPTIONS+=' --release'
        QTWEBKIT_OPTIONS+=" --qmakearg=CONFIG+=static"
        QTWEBKIT_OPTIONS+=" --qmakearg=DEFINES+=Q_NODLL"
        QTWEBKIT_OPTIONS+=" --qmakearg=DEFINES+=STATIC"
    
        QTWEBKIT_OPTIONS+=' --no-webkit2'
        QTWEBKIT_OPTIONS+=' --no-3d-rendering'
        QTWEBKIT_OPTIONS+=' --no-webgl'
        QTWEBKIT_OPTIONS+=' --no-gamepad'
        QTWEBKIT_OPTIONS+=' --no-video'
        QTWEBKIT_OPTIONS+=' --no-xslt'
        QTWEBKIT_OPTIONS+=" --qmakearg=DEFINES+=WTF_USE_3D_GRAPHICS=0"
        QTWEBKIT_OPTIONS+=" --qmakearg=DEFINES+=WTF_USE_ZLIB=0"

        if [[ $OSTYPE == cygwin ]]; then export QMAKEPATH="$QTDIR/webkit-qtwebkit-23/Tools/qmake"; fi
    
        Tools/Scripts/build-webkit $QTWEBKIT_OPTIONS || qtwebkit_error
        pushd WebKitBuild/Release
        $MAKE_COMMAND install
        popd
        popd
    fi
    
    #Revert patches to clean sources
    for i in "${QT_PATCHES[@]}"; do
        patch -p0 -R < "$PATCHDIR/$i"
    done
    
    for k in "${QTWEBKIT_PATCHES[@]}"; do
        patch -p0 -R < "$PATCHDIR/$k"
    done
    popd
fi # end of Qt build

QMAKE_ARGS=
if $USE_QTWEBKIT_23; then QMAKE_ARGS="DEFINES+=QTWEBKIT_23"; fi
$MAKE_COMMAND distclean
src/qt/bin/qmake "$QMAKE_ARGS" -r -config release 
$MAKE_COMMAND

#fix qt_menu.nib issue
if [[ $OSTYPE = darwin* ]]; then
    cp -r src/qt/src/gui/mac/qt_menu.nib build/QtWeb.app/Contents/Resources/
fi

exit 0
