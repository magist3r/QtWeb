call env.cmd

cd src\qt\
..\..\bin\patch.exe -p0 -N < ..\..\qt-patches\0001-configure.patch
..\..\bin\patch.exe -p0 -N < ..\..\qt-patches\0002-webkit-pro.patch
..\..\bin\patch.exe -p0 -N < ..\..\qt-patches\0003-qtwebkit-pro.patch
..\..\bin\patch.exe -p0 -N < ..\..\qt-patches\0004-qstyles-qrc.patch
..\..\bin\patch.exe -p0 -N < ..\..\qt-patches\0005-qwidget-cpp.patch
..\..\bin\patch.exe -p0 -N < ..\..\qt-patches\0006-webkit-disable-npapi.patch
..\..\bin\patch.exe -p0 -N < ..\..\qt-patches\0011-windows-mkspec.patch
..\..\bin\patch.exe -p0 -N < ..\..\qt-patches\0012-windows-webcore-pro.patch
..\..\bin\patch.exe -p0 -N < ..\..\qt-patches\0013-windows-dotnet-style.patch

set OPTIONS=
set OPTIONS=%OPTIONS% -opensource
set OPTIONS=%OPTIONS% -confirm-license
set OPTIONS=%OPTIONS% -static
set OPTIONS=%OPTIONS% -webkit

set OPTIONS=%OPTIONS% -release
set OPTIONS=%OPTIONS% -fast
set OPTIONS=%OPTIONS% -nomake demos
set OPTIONS=%OPTIONS% -nomake docs
set OPTIONS=%OPTIONS% -nomake examples
set OPTIONS=%OPTIONS% -nomake translations
set OPTIONS=%OPTIONS% -nomake tests
set OPTIONS=%OPTIONS% -nomake tools
set OPTIONS=%OPTIONS% -no-declarative
set OPTIONS=%OPTIONS% -no-multimedia
set OPTIONS=%OPTIONS% -no-opengl
set OPTIONS=%OPTIONS% -no-openvg
set OPTIONS=%OPTIONS% -no-phonon
set OPTIONS=%OPTIONS% -no-qt3support
set OPTIONS=%OPTIONS% -no-script
set OPTIONS=%OPTIONS% -no-scripttools
set OPTIONS=%OPTIONS% -qt-libjpeg
set OPTIONS=%OPTIONS% -qt-libpng
set OPTIONS=%OPTIONS% -qt-zlib
set OPTIONS=%OPTIONS% -openssl-linked OPENSSL_LIBS="-lssleay32 -llibeay32 -lcrypt32 -lgdi32"
set OPTIONS=%OPTIONS% -no-dbus
set OPTIONS=%OPTIONS% -no-stl
set OPTIONS=%OPTIONS% -no-libtiff
set OPTIONS=%OPTIONS% -no-libmng
set OPTIONS=%OPTIONS% -no-style-motif
set OPTIONS=%OPTIONS% -no-style-cde
set OPTIONS=%OPTIONS% -no-s60
set OPTIONS=%OPTIONS% -mp
set OPTIONS=%OPTIONS% -no-accessibility

configure -prefix %cd% %OPTIONS% && nmake
..\..\bin\patch.exe -p0 -R < ..\..\qt-patches\0001-configure.patch
..\..\bin\patch.exe -p0 -R < ..\..\qt-patches\0002-webkit-pro.patch
..\..\bin\patch.exe -p0 -R < ..\..\qt-patches\0003-qtwebkit-pro.patch
..\..\bin\patch.exe -p0 -R < ..\..\qt-patches\0004-qstyles-qrc.patch
..\..\bin\patch.exe -p0 -R < ..\..\qt-patches\0005-qwidget-cpp.patch
..\..\bin\patch.exe -p0 -R < ..\..\qt-patches\0006-webkit-disable-npapi.patch
..\..\bin\patch.exe -p0 -R < ..\..\qt-patches\0012-windows-webcore-pro.patch
..\..\bin\patch.exe -p0 -R < ..\..\qt-patches\0013-windows-dotnet-style.patch
src\qt\bin\qmake.exe -config release && nmake clean && nmake
