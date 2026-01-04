QT       += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

VERSION = 1.0.12.11
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$PWD/../../../src/VeriQC_UI/VeriQC_UI.pri)

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


contains(DEFINES,CONSOLE){
CONFIG += console
TARGET = VeriQC_Console
DEFINES += QT_NO_OPENGL
}

win32 {
CONFIG(debug,debug|release){
LIBS += -L../exe/Debug64 -lVeriQC
DESTDIR = $$PWD/../bin/Debug
}else{
LIBS += -L../exe/Release64 -lVeriQC
DESTDIR = $$PWD/../bin/Release
}
} else {

CONFIG(debug,debug|release){
LIBS += -L$$PWD/../build_VeriQC_Lib -lVeriQC
DESTDIR = $$PWD/../bin/Debug
}else{
LIBS += -L$$PWD/../build_VeriQC_Lib -lVeriQC
DESTDIR = $$PWD/../bin/Release
}
}

win32 {
    RC_ICONS = $$PWD/../../../src/VeriQC_UI/Resources/Icons/VeriQC-LOGO.ico
    QMAKE_TARGET_COMPANY = "CHCNAV"
    QMAKE_TARGET_PRODUCT = "VariQC"
    QMAKE_TARGET_COPYRIGHT = "Copyright:Shanghai Huace Navigation Technology Ltd. All right reserved."
} else {
QMAKE_LFLAGS += -no-pie
}
