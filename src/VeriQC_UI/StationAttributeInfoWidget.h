/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file StationAttributeInfoWidget.h
 * @author CHC
 * @date 2025-09-28
 * @brief Widget for show QC Object station attribute information 
 * 
**************************************************************************/
#ifndef STATIONATTRIBUTEINFOWIDGET_H
#define STATIONATTRIBUTEINFOWIDGET_H

#include <QWidget>
#include "QCObject.h"

namespace Ui {
class StationAttributeInfoWidget;
}

class StationAttributeInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StationAttributeInfoWidget(QWidget *parent = nullptr);
    ~StationAttributeInfoWidget();
    void updateInfo(QSharedPointer<QCObject> qcObject);
    void hideWidget();
    void retranslateUi();
signals:
    void inputFormatChanged(int qcObjectId);
protected:
    void initInfoWidget(QWidget *widget);
    void showEvent(QShowEvent *event) override;
    void onStationInfoToolButtonClicked(bool checked);
    void onObsInfoToolButtonClicked(bool checked);
    void onRecevierInfoToolButtonClicked(bool checked);
    void onAntennaInfoToolButtonClicked(bool checked);
    void onSwitchInputFormat();
private:
    Ui::StationAttributeInfoWidget *ui;
    QSharedPointer<QCObject> m_qcObject = nullptr;
};

#endif // STATIONATTRIBUTEINFOWIDGET_H
