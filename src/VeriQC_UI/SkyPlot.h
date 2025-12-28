/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI plot Module
 * *-
 * @file SkyPlot.h
 * @author CHC
 * @date 2025-09-28
 * @brief plot for draw satellite sky map
 * 
**************************************************************************/
#ifndef SKYPLOT_H
#define SKYPLOT_H

#include "Components/MyProgressCustomPlot.h"
#include "SkyPlotPolarSatGraph.h"
#include "SkyPlotSatPolarAxisAngular.h"

class SkyPlot : public MyProgressCustomPlot
{
    Q_OBJECT
public:
    SkyPlot(int qcObjectId, QWidget *parent = nullptr);
    const QCPColorGradient &getColorGradient()const;
    void setSysVisible(QCResultManager::QCSysTypeEnum sys, bool visible);
    void updatePlot();
protected:
    QColor getSNRColor(double value);
    virtual void doRun() override;
    QCPLayer *createSysLayer(const QString &layerName);
private:
    QCPColorGradient m_snrColorGradient;
    int m_qcObjectId;
    SkyPlotSatPolarAxisAngular *m_angularAxis;
    QMap<QCResultManager::QCSysTypeEnum,SkyPlotPolarSatGraph *> m_sysGraphs;
    QMap<QCResultManager::QCSysTypeEnum,bool> m_sysVisibleMap;
    QMap<QCResultManager::QCSysTypeEnum,QCPLayer*> m_sysLayers;
};

#endif // SKYPLOT_H
