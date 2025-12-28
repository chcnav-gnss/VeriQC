/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI plot Module
 * *-
 * @file SkyPlotSatPolarAxisAngular.h
 * @author CHC
 * @date 2025-09-28
 * @brief Axis angular of sky plot
 * 
**************************************************************************/
#ifndef SKYPLOTSATPOLARAXISANGULAR_H
#define SKYPLOTSATPOLARAXISANGULAR_H

#include "QCustomplot/qcustomplot.h"

class SkyPlotSatPolarAxisAngular : public QCPPolarAxisAngular
{
    Q_OBJECT
public:
    SkyPlotSatPolarAxisAngular(QCustomPlot *parentPlot);
};

#endif // SKYPLOTSATPOLARAXISANGULAR_H
