#!/bin/bash
PATCHDIR="$(pwd)/qt-patches"
SSL_LIBS=''
SKIP_QT_BUILD=false
CLEAN_QT_BUILD=false
CROSS_COMPILE=false
CROSS_COMPILE_PREFIX='i686-w64-mingw32-'
COMPILE_JOBS=8
MAKE_COMMAND="make -j$COMPILE_JOBS"
PATCHES=() # Array of patches
MKSPEC_PATCHES=() # Patches for qmake.conf 

PATCHES+=('0001-configure.patch')
PATCHES+=('0002-webkit-pro.patch')
PATCHES+=('0003-qtwebkit-pro.patch')
PATCHES+=('0004-qstyles-qrc.patch')
PATCHES+=('0005-qwidget-cpp.patch')
PATCHES+=('0006-webkit-disable-npapi.patch')

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
    OPTIONS+=' -silent'
    OPTIONS+=' -fast'

    if [[ $OSTYPE = darwin* ]]; then
        OPTIONS+=' -arch x86'
        OPTIONS+=' -carbon' # use carbon for compatibility reasons
        OPTIONS+=' -openssl'
        OPTIONS+=' -sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk'
        
        PATCHES+=('0031-mac-fix-includepath-for-npapi.patch')
    elif [[ $OSTYPE = msys ]]; then
        SSL_LIBS='-lssleay32 -llibeay32 -lcrypt32 -lgdi32'
        OPTIONS+=' -openssl-linked'
        OPTIONS+=' -mp'
        OPTIONS+=' -no-s60'
        MAKE_COMMAND=nmake

        MKSPEC_PATCHES+=('0011-windows-mkspec.patch')
        
        PATCHES+=('0012-windows-webcore-pro.patch')
        PATCHES+=('0013-windows-dotnet-style.patch')
    elif [[ $OSTYPE = beos ]]; then
        OPTIONS+=' -no-largefile'
        OPTIONS+=' -no-pch'
        OPTIONS+=' -openssl'
    else
        if $CROSS_COMPILE; then
            OPTIONS+=' -xplatform win32-g++'
            OPTIONS+=" -device-option CROSS_COMPILE=$CROSS_COMPILE_PREFIX"
            SSL_LIBS='-lssl -lcrypto -lcrypt32 -lgdi32'
            OPTIONS+=' -openssl-linked'
            
            PATCHES+=('0012-windows-webcore-pro.patch')
            PATCHES+=('0013-windows-dotnet-style.patch')
            
            MKSPEC_PATCHES+=('0014-windows-mkspec-cross-compile.patch')
        else
            OPTIONS+=' -system-freetype'
            OPTIONS+=' -fontconfig'
            OPTIONS+=' -reduce-relocations'
            OPTIONS+=' -openssl'
            
            MKSPEC_PATCHES+=('0021-linux-mkspec.patch')
            
            PATCHES+=('0022-linux-qgtkstyle-qtbug-23569.patch')
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
    OPTIONS+=' -optimized-qmake'

    cd src/qt
    if [ ! -e ./configure ]; then
        echo "Looks like you forgot to extract Qt sources in src/qt directory!"
        exit 1
    fi
    
    # make clean if we have previous build in src/qt
    if $CLEAN_QT_BUILD; then
        make confclean
    fi
    
    #Applying patches for Qt
    for i in "${PATCHES[@]}"; do
        patch -p0 -N < "$PATCHDIR/$i"
    done
    
    for j in "${MKSPEC_PATCHES[@]}"; do
        patch -p0 -N < "$PATCHDIR/$j"
    done

    OPENSSL_LIBS="$SSL_LIBS" ./configure -prefix $PWD $OPTIONS && $MAKE_COMMAND || qt_error
    
    #Revert patches to clean sources
    for i in "${PATCHES[@]}"; do
        patch -p0 -R < "$PATCHDIR/$i"
    done

    cd ../..
fi # end of Qt build

src/qt/bin/qmake -config release
make clean
$MAKE_COMMAND

#fix qt_menu.nib issue
if [[ $OSTYPE = darwin* ]]; then
    cd ..
    cp -r src/qt/src/gui/mac/qt_menu.nib build/QtWeb.app/Contents/Resources/
fi

exit 0
