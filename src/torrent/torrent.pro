HEADERS += addtorrentdialog.h \
           bencodeparser.h \
           connectionmanager.h \
           metainfo.h \
           peerwireclient.h \
           ratecontroller.h \
           filemanager.h \
           torrentclient.h \
           torrentserver.h \
           torrentwindow.h \
           trackerclient.h

SOURCES += addtorrentdialog.cpp \
           bencodeparser.cpp \
           connectionmanager.cpp \
           metainfo.cpp \
           peerwireclient.cpp \
           ratecontroller.cpp \
           filemanager.cpp \
           torrentclient.cpp \
           torrentserver.cpp \
           torrentwindow.cpp \
           trackerclient.cpp

# Forms and resources
FORMS += forms/addtorrentform.ui
RESOURCES += icons.qrc

QT += network widgets
