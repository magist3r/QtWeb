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
set OPTIONS=%OPTIONS% -no-scripttools
set OPTIONS=%OPTIONS% -qt-libjpeg
set OPTIONS=%OPTIONS% -qt-libpng
set OPTIONS=%OPTIONS% -qt-zlib
set OPTIONS=%OPTIONS% -openssl
:: set OPTIONS=%OPTIONS% -openssl-linked OPENSSL_LIBS="-lssleay32 -llibeay32 -luser32 -lgdi32"
set OPTIONS=%OPTIONS% -no-dbus
set OPTIONS=%OPTIONS% -no-stl
set OPTIONS=%OPTIONS% -no-libtiff
set OPTIONS=%OPTIONS% -no-libmng
set OPTIONS=%OPTIONS% -no-style-plastique
set OPTIONS=%OPTIONS% -no-style-motif
set OPTIONS=%OPTIONS% -no-style-cde
set OPTIONS=%OPTIONS% -no-style-cleanlooks
set OPTIONS=%OPTIONS% -no-s60
set OPTIONS=%OPTIONS% -mp


configure %OPTIONS%


