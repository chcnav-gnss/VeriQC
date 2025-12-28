
HEADERS += \
    $$PWD/HttpClient.h \
    $$PWD/UpgradeMainWindow.h \
    $$PWD/ButtonHoverWatcher.h \
    $$PWD/framelesswindow.h \
    $$PWD/zip/CZipExtractor.h
SOURCES += \
    $$PWD/ButtonHoverWatcher.cpp \
    $$PWD/framelesswindow.cpp \
    $$PWD/HttpClient.cpp \
    $$PWD/UpgradeMainWindow.cpp \
    $$PWD/main.cpp \
    $$PWD/zip/CZipExtractor.cpp
RESOURCES += $$PWD/AutoUpgradePlug.qrc
TRANSLATIONS += \
    $$PWD/Translation_Upgrade_zh.ts
