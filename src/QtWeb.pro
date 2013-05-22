TEMPLATE = app
TARGET = QtWeb
DESTDIR = ../build
QT += network xml webkit
CONFIG += static
QTPLUGIN += qcncodecs qjpcodecs qkrcodecs qtwcodecs qico 
DEFINES += QT_NO_UITOOLS

INCLUDEPATH += $$DESTDIR/moc \
    $$DESTDIR/rcc \
    $$DESTDIR/uic 
    
MOC_DIR = $$DESTDIR/moc/
OBJECTS_DIR = $$DESTDIR/obj/
UI_DIR = $$DESTDIR/uic/
RCC_DIR = $$DESTDIR/rcc/

#Windows resource file
win32:RC_FILE = QtWeb.rc

macx {
    QMAKE_CXXFLAGS = -fvisibility=hidden -fvisibility-inlines-hidden
    ICON = qtweb.icns
}

VPATH += torrent
INCLUDEPATH += torrent
include(torrent/torrent.pro)

HEADERS += aboutdialog.h \
    autocomplete.h \
    autosaver.h \
    bookmarks.h \
    bookmarksimport.h \
    browserapplication.h \
    browsermainwindow.h \
    certificateinfo.h \
    chasewidget.h \
    closeapp.h \
    commands.h \
    cookiejar.h \
    downloadmanager.h \
    edittableview.h \
    edittreeview.h \
    exlineedit.h \
    googlesuggest.h \
    history.h \
    findwidget.h \
    modelmenu.h \
    networkaccessmanager.h \
    passwords.h \
    resetsettings.h \
    savepdf.h \
    searches.h \
    searchlineedit.h \
    settings.h \
    shortcuts.h \
    squeezelabel.h \
    tabbar.h \
    tabwidget.h \
    toolbarsearch.h \
    urllineedit.h \
    viewsource.h \
    webpage.h \
    webview.h \
    xbel.h

SOURCES += aboutdialog.cpp \
    autocomplete.cpp \
    autosaver.cpp \
    bookmarks.cpp \
    bookmarksimport.cpp \
    browserapplication.cpp \
    browsermainwindow.cpp \
    certificateinfo.cpp \
    chasewidget.cpp \
    closeapp.cpp \
    commands.cpp \
    cookiejar.cpp \
    downloadmanager.cpp \
    edittableview.cpp \
    edittreeview.cpp \
    exlineedit.cpp \
    googlesuggest.cpp \
    history.cpp \
    findwidget.cpp \
    main.cpp \
    modelmenu.cpp \
    networkaccessmanager.cpp \
    passwords.cpp \
    resetsettings.cpp \
    savepdf.cpp \
    searches.cpp \
    searchlineedit.cpp \
    settings.cpp \
    shortcuts.cpp \
    squeezelabel.cpp \
    tabbar.cpp \
    tabwidget.cpp \
    toolbarsearch.cpp \
    urllineedit.cpp \
    viewsource.cpp \
    webpage.cpp \
    webview.cpp \
    xbel.cpp

FORMS += aboutdialog.ui \
    addbookmarkdialog.ui \
    bookmarks.ui \
    certificateinfo.ui \
    closeapp.ui \
    cookies.ui \
    cookiesexceptions.ui \
    downloaditem.ui \
    downloads.ui \
    history.ui \
    master.ui \
    passworddialog.ui \
    passwords.ui \
    proxy.ui \
    resetsettings.ui \
    savepdf.ui \
    search.ui \
    searches.ui \
    settings.ui \
    shortcuts.ui

RESOURCES += data/data.qrc \
    htmls/htmls.qrc 
