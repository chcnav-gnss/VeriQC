
INCLUDEPATH += $$PWD/../../../VeriQC/src
INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/AboutDialog.cpp \
    $$PWD/AppSettingsManagement.cpp \
    $$PWD/CommandLineProcessor.cpp \
    $$PWD/Components/MyAlertMessageBox.cpp \
    $$PWD/Components/MyComboBox.cpp \
    $$PWD/Components/MyDateTimeEdit.cpp \
    $$PWD/Components/MyDialog.cpp \
    $$PWD/Components/MyHeaderView.cpp \
    $$PWD/Components/MyMenu.cpp \
    $$PWD/Components/MyMessageBox.cpp \
    $$PWD/Components/MyNoWheelDoubleSpinBox.cpp \
    $$PWD/Components/MyNoWheelSpinBox.cpp \
    $$PWD/Components/MyProgressCustomPlot.cpp \
    $$PWD/Components/MyWaterProgressBar.cpp \
    $$PWD/ExpandButtonDelegate.cpp \
    $$PWD/ImportFileDialog.cpp \
    $$PWD/MainWindow.cpp \
    $$PWD/ObsTimePlot.cpp \
    $$PWD/ObsTimePlotDialog.cpp \
    $$PWD/PlotTimeAxisTicker.cpp \
    $$PWD/QCConfigDialog.cpp \
    $$PWD/QCConfigurator.cpp \
    $$PWD/QCObject.cpp \
    $$PWD/QCObjectManager.cpp \
    $$PWD/QCObjectTableModel.cpp \
    $$PWD/QCProcessThread.cpp \
    $$PWD/QCProcessor.cpp \
    $$PWD/QCProgressDialog.cpp \
    $$PWD/QCResultExporter.cpp \
    $$PWD/QCResultManager.cpp \
    $$PWD/QCustomplot/qcustomplot.cpp \
    $$PWD/RinexConvertSettingDialog.cpp \
    $$PWD/RinexMergeDialog.cpp \
    $$PWD/RinexMergeProcessor.cpp \
    $$PWD/SNRAndMPTimePlot.cpp \
    $$PWD/SNRAndMPTimePlotDialog.cpp \
    $$PWD/SatNumAndDOPTimePlot.cpp \
    $$PWD/SatNumAndDOPTimePlotDialog.cpp \
    $$PWD/SkyPlot.cpp \
    $$PWD/SkyPlotColorBarWidget.cpp \
    $$PWD/SkyPlotDialog.cpp \
    $$PWD/SkyPlotPolarSatGraph.cpp \
    $$PWD/SkyPlotSatPolarAxisAngular.cpp \
    $$PWD/StationAttributeInfoWidget.cpp \
    $$PWD/TimePlotGraph.cpp \
    $$PWD/main.cpp \
    $$PWD/upgrade/CNewVersionDescDialog.cpp \
    $$PWD/upgrade/HttpClient.cpp \
    $$PWD/upgrade/UpgradeHelp.cpp

HEADERS += \
    $$PWD/AboutDialog.h \
    $$PWD/AppSettingsManagement.h \
    $$PWD/CommandLineProcessor.h \
    $$PWD/Components/MyAlertMessageBox.h \
    $$PWD/Components/MyComboBox.h \
    $$PWD/Components/MyDateTimeEdit.h \
    $$PWD/Components/MyDialog.h \
    $$PWD/Components/MyHeaderView.h \
    $$PWD/Components/MyMenu.h \
    $$PWD/Components/MyMessageBox.h \
    $$PWD/Components/MyNoWheelDoubleSpinBox.h \
    $$PWD/Components/MyNoWheelSpinBox.h \
    $$PWD/Components/MyProgressCustomPlot.h \
    $$PWD/Components/MyWaterProgressBar.h \
    $$PWD/ExpandButtonDelegate.h \
    $$PWD/ImportFileDialog.h \
    $$PWD/MainWindow.h \
    $$PWD/ObsTimePlot.h \
    $$PWD/ObsTimePlotDialog.h \
    $$PWD/PlotTimeAxisTicker.h \
    $$PWD/QCConfigDialog.h \
    $$PWD/QCConfigurator.h \
    $$PWD/QCObject.h \
    $$PWD/QCObjectManager.h \
    $$PWD/QCObjectTableModel.h \
    $$PWD/QCProcessThread.h \
    $$PWD/QCProcessor.h \
    $$PWD/QCProgressDialog.h \
    $$PWD/QCResultExporter.h \
    $$PWD/QCResultManager.h \
    $$PWD/QCustomplot/qcustomplot.h \
    $$PWD/RinexConvertSettingDialog.h \
    $$PWD/RinexMergeDialog.h \
    $$PWD/RinexMergeProcessor.h \
    $$PWD/SNRAndMPTimePlot.h \
    $$PWD/SNRAndMPTimePlotDialog.h \
    $$PWD/SatNumAndDOPTimePlot.h \
    $$PWD/SatNumAndDOPTimePlotDialog.h \
    $$PWD/SkyPlot.h \
    $$PWD/SkyPlotColorBarWidget.h \
    $$PWD/SkyPlotDialog.h \
    $$PWD/SkyPlotPolarSatGraph.h \
    $$PWD/SkyPlotSatPolarAxisAngular.h \
    $$PWD/StationAttributeInfoWidget.h \
    $$PWD/TimePlotGraph.h \
    $$PWD/upgrade/CNewVersionDescDialog.h \
    $$PWD/upgrade/HttpClient.h \
    $$PWD/upgrade/UpgradeDTO.h \
    $$PWD/upgrade/UpgradeHelp.h

FORMS += \
    $$PWD/AboutDialog.ui \
    $$PWD/ImportFileDialog.ui \
    $$PWD/MainWindow.ui \
    $$PWD/ObsTimePlotDialog.ui \
    $$PWD/QCConfigDialog.ui \
    $$PWD/QCProgressDialog.ui \
    $$PWD/RinexConvertSettingDialog.ui \
    $$PWD/RinexMergeDialog.ui \
    $$PWD/SNRAndMPTimePlotDialog.ui \
    $$PWD/SatNumAndDOPTimePlotDialog.ui \
    $$PWD/SkyPlotColorBarWidget.ui \
    $$PWD/SkyPlotDialog.ui \
    $$PWD/StationAttributeInfoWidget.ui \
    $$PWD/upgrade/CNewVersionDescDialog.ui

TRANSLATIONS += \
    $$PWD/VeriQC_UI_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    $$PWD/Resource.qrc

DISTFILES += \
    $$PWD/VeriQC_UI_zh_CN.ts

QXLSX_PARENTPATH=$$PWD/QXlsx/
QXLSX_HEADERPATH=$$PWD/QXlsx/header/
QXLSX_SOURCEPATH=$$PWD/QXlsx/source/
include($$PWD/QXlsx/QXlsx.pri)
