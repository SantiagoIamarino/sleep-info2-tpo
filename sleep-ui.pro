QT       += core gui sql charts widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    src/logic/profiledao.cpp \
    src/ui/liveclient/liveclient.cpp \
    src/ui/mainwindow.cpp \
    src/ui/pagehistorial/pagehistorial.cpp \
    src/ui/pageajustes/pageajustes.cpp \
    src/ui/profileselector.cpp \
    src/ui/profilecreator.cpp \
    src/utils/database.cpp

HEADERS += \
    src/logic/profiledao.h \
    src/models/profile.h \
    src/ui/liveclient/liveclient.h \
    src/ui/mainwindow.h \
    src/ui/pagehistorial/pagehistorial.h \
    src/ui/pageajustes/pageajustes.h \
    src/ui/profileselector.h \
    src/ui/profilecreator.h \
    src/utils/database.h

FORMS += \
    src/ui/mainwindow.ui \
    src/ui/profileselector.ui \
    src/ui/profilecreator.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
