#include "SkyPlotPolarSatGraph.h"

SkyPlotPolarSatGraph::SkyPlotPolarSatGraph(QCPPolarAxisAngular *keyAxis, QCPPolarAxisRadial *valueAxis)
    :QCPPolarGraph(keyAxis,valueAxis)
{
    m_skyDataContainer = QSharedPointer<SkyGraphDataContainer>(new SkyGraphDataContainer());
    this->setAntialiased(false);
    this->setAntialiasedScatters(false);
}

void SkyPlotPolarSatGraph::addData(const SkyGraphData &data)
{
    m_skyDataContainer->add(data);
    m_latestSatPointData[data.satName] = data;
}

void SkyPlotPolarSatGraph::clearData()
{
    m_skyDataContainer->clear();
    m_latestSatPointData.clear();
}

double SkyPlotPolarSatGraph::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if ((onlySelectable && mSelectable == QCP::stNone) || m_skyDataContainer->isEmpty())
    return -1;
  if (!mKeyAxis || !mValueAxis)
    return -1;
  
  if (mKeyAxis->rect().contains(pos.toPoint()))
  {
    SkyGraphDataContainer::const_iterator closestDataPoint = m_skyDataContainer->constEnd();
    double result = pointDistance(pos, closestDataPoint);
    if (details)
    {
      int pointIndex = closestDataPoint-m_skyDataContainer->constBegin();
      details->setValue(QCPDataSelection(QCPDataRange(pointIndex, pointIndex+1)));
    }
    return result;
  } else
    return -1;
}

QCPRange SkyPlotPolarSatGraph::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain, const QCPRange &inKeyRange) const
{
  return m_skyDataContainer->valueRange(foundRange, inSignDomain, inKeyRange);
}

QCPRange SkyPlotPolarSatGraph::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
  return m_skyDataContainer->keyRange(foundRange, inSignDomain);
}

void SkyPlotPolarSatGraph::getPlotScatters(QVector<SkyPlotGraphPointData> *scatters, const QCPDataRange &dataRange) const
{
  QCPPolarAxisAngular *keyAxis = mKeyAxis.data();
  QCPPolarAxisRadial *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }

  if (!scatters) return;
  SkyGraphDataContainer::const_iterator begin, end;
  getVisibleDataBounds(begin, end, dataRange);
  if (begin == end)
  {
    scatters->clear();
    return;
  }

  QVector<SkyGraphData> data;
  getOptimizedScatterData(&data, begin, end);

  scatters->resize(data.size());
  for (int i=0; i<data.size(); ++i)
  {
    if (!qIsNaN(data.at(i).value))
    {
        QPointF point = valueAxis->coordToPixel(data.at(i).key, data.at(i).value);
        (*scatters)[i].point = point.toPoint();
        (*scatters)[i].color = data.at(i).color;
    }
  }
}

double SkyPlotPolarSatGraph::pointDistance(const QPointF &pixelPoint, SkyGraphDataContainer::const_iterator &closestData) const
{
  closestData = m_skyDataContainer->constEnd();
  if (m_skyDataContainer->isEmpty())
    return -1.0;
  if (mLineStyle == lsNone && mScatterStyle.isNone())
    return -1.0;
  
  // calculate minimum distances to graph data points and find closestData iterator:
  double minDistSqr = (std::numeric_limits<double>::max)();
  // determine which key range comes into question, taking selection tolerance around pos into account:
  double posKeyMin, posKeyMax, dummy;
  pixelsToCoords(pixelPoint-QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMin, dummy);
  pixelsToCoords(pixelPoint+QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMax, dummy);
  if (posKeyMin > posKeyMax)
    qSwap(posKeyMin, posKeyMax);
  // iterate over found data points and then choose the one with the shortest distance to pos:
  SkyGraphDataContainer::const_iterator begin = m_skyDataContainer->findBegin(posKeyMin, true);
  SkyGraphDataContainer::const_iterator end = m_skyDataContainer->findEnd(posKeyMax, true);
  for (SkyGraphDataContainer::const_iterator it=begin; it!=end; ++it)
  {
    const double currentDistSqr = QCPVector2D(coordsToPixels(it->key, it->value)-pixelPoint).lengthSquared();
    if (currentDistSqr < minDistSqr)
    {
      minDistSqr = currentDistSqr;
      closestData = it;
    }
  }
    
  // calculate distance to graph line if there is one (if so, will probably be smaller than distance to closest data point):
  if (mLineStyle != lsNone)
  {
    // line displayed, calculate distance to line segments:
    QVector<QPointF> lineData;
    getLines(&lineData, QCPDataRange(0, dataCount()));
    QCPVector2D p(pixelPoint);
    for (int i=0; i<lineData.size()-1; ++i)
    {
      const double currentDistSqr = p.distanceSquaredToLine(lineData.at(i), lineData.at(i+1));
      if (currentDistSqr < minDistSqr)
        minDistSqr = currentDistSqr;
    }
  }
  
  return qSqrt(minDistSqr);
}

int SkyPlotPolarSatGraph::dataCount() const
{
  return m_skyDataContainer->size();
}

void SkyPlotPolarSatGraph::getVisibleDataBounds(SkyGraphDataContainer::const_iterator &begin, SkyGraphDataContainer::const_iterator &end, const QCPDataRange &rangeRestriction) const
{
  if (rangeRestriction.isEmpty())
  {
    end = m_skyDataContainer->constEnd();
    begin = end;
  } else
  {
    QCPPolarAxisAngular *keyAxis = mKeyAxis.data();
    QCPPolarAxisRadial *valueAxis = mValueAxis.data();
    if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
    // get visible data range:
    if (mPeriodic)
    {
      begin = m_skyDataContainer->constBegin();
      end = m_skyDataContainer->constEnd();
    } else
    {
      begin = m_skyDataContainer->findBegin(keyAxis->range().lower);
      end = m_skyDataContainer->findEnd(keyAxis->range().upper);
    }
    // limit lower/upperEnd to rangeRestriction:
    m_skyDataContainer->limitIteratorsToDataRange(begin, end, rangeRestriction); // this also ensures rangeRestriction outside data bounds doesn't break anything
  }
}

void SkyPlotPolarSatGraph::drawPlotScatterPlot(QCPPainter *painter, const QVector<SkyPlotGraphPointData> &scatters, const QCPScatterStyle &style) const
{
    QCPPolarAxisRadial *valueAxis = mValueAxis.data();
    applyScattersAntialiasingHint(painter);
    style.applyTo(painter, mPen);

    QPoint centerPoint = valueAxis->coordToPixel(0,90).toPoint();
    QPoint topPoint = valueAxis->coordToPixel(0,0).toPoint();
    int radius = abs(topPoint.y() - centerPoint.y());
    int left = centerPoint.x() - radius - 1;
    int right = centerPoint.x() + radius + 1;
    int top = topPoint.y() - 1;
    int bottom = centerPoint.y() + radius + 1;

    QVector<bool> pixelMap(abs((right - left + 1) * (bottom - top + 1)));


    int scattersSize = scatters.size();
    painter->save();
    if(scattersSize > 0)
    {
      for (int i=0; i< scattersSize - 1; ++i)
      {
          int pixelIndex = (scatters[i].point.x() - left) + (scatters[i].point.y() - top) * (right - left + 1);
          if(pixelIndex >= 0)
          {
              if(pixelMap[pixelIndex])
              {
                  continue;     // ignore pixel points that has drawn
              }
            pixelMap[pixelIndex] = true;
          }
          QPen pen = painter->pen();
          pen.setColor(scatters.at(i).color);
          painter->setPen(pen);
          style.drawShape(painter, scatters.at(i).point.x(), scatters.at(i).point.y());
      }

      for(const SkyGraphData &satData : m_latestSatPointData.values())
      {
        QPointF point = valueAxis->coordToPixel(satData.key, satData.value);

        // the latest point draw sat name
        int radius = 10;
//        QCPScatterStyle latestStyle = style;
//        latestStyle.setSize(radius*2);
//        painter->setPen(satData.color);
//        latestStyle.drawShape(painter,point.x(),point.y());
        QFont satNameFont;
        satNameFont.setBold(true);
        satNameFont.setPixelSize(radius);
        QRectF nameRect = painter->fontMetrics().boundingRect(
                    point.x() - radius,
                    point.y() - radius,
                    radius *2,radius*2,Qt::AlignCenter,satData.satName);
        painter->setFont(satNameFont);
        painter->setPen(QColor(Qt::black));
        painter->drawText(nameRect,Qt::AlignCenter,satData.satName);

      }
    }
    painter->restore();
}

void SkyPlotPolarSatGraph::draw(QCPPainter *painter)
{
  if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  if (mKeyAxis.data()->range().size() <= 0 || m_skyDataContainer->isEmpty()) return;
  if (mLineStyle == lsNone && mScatterStyle.isNone()) return;

  painter->setClipRegion(mKeyAxis->exactClipRegion());

  QVector<QPointF> lines; // line and (if necessary) scatter pixel coordinates will be stored here while iterating over segments

  // loop over and draw segments of unselected/selected data:
  QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  allSegments << unselectedSegments << selectedSegments;
  for (int i=0; i<allSegments.size(); ++i)
  {
    bool isSelectedSegment = i >= unselectedSegments.size();
    // get line pixel points appropriate to line style:
    QCPDataRange lineDataRange = isSelectedSegment ? allSegments.at(i) : allSegments.at(i).adjusted(-1, 1); // unselected segments extend lines to bordering selected data point (safe to exceed total data bounds in first/last segment, getLines takes care)
    getLines(&lines, lineDataRange);

    // check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA
    SkyGraphDataContainer::const_iterator it;
    for (it = m_skyDataContainer->constBegin(); it != m_skyDataContainer->constEnd(); ++it)
    {
      if (QCP::isInvalidData(it->key, it->value))
        qDebug() << Q_FUNC_INFO << "Data point at" << it->key << "invalid." << "Plottable name:" << name();
    }
#endif

    // draw fill of graph:
    //if (isSelectedSegment && mSelectionDecorator)
    //  mSelectionDecorator->applyBrush(painter);
    //else
      painter->setBrush(mBrush);
    painter->setPen(Qt::NoPen);
    drawFill(painter, &lines);


    // draw line:
    if (mLineStyle != lsNone)
    {
      //if (isSelectedSegment && mSelectionDecorator)
      //  mSelectionDecorator->applyPen(painter);
      //else
        painter->setPen(mPen);
      painter->setBrush(Qt::NoBrush);
      drawLinePlot(painter, lines);
    }
    // draw scatters:

    QCPScatterStyle finalScatterStyle = mScatterStyle;
    //if (isSelectedSegment && mSelectionDecorator)
    //  finalScatterStyle = mSelectionDecorator->getFinalScatterStyle(mScatterStyle);
    if (!finalScatterStyle.isNone())
    {
      QVector<SkyPlotGraphPointData> scatters;
      getPlotScatters(&scatters, allSegments.at(i));
      drawPlotScatterPlot(painter, scatters, finalScatterStyle);
    }
  }

  // draw other selection decoration that isn't just line/scatter pens and brushes:
  //if (mSelectionDecorator)
  //  mSelectionDecorator->drawDecoration(painter, selection());
}

void SkyPlotPolarSatGraph::getOptimizedScatterData(QVector<SkyGraphData> *scatterData, SkyGraphDataContainer::const_iterator begin, SkyGraphDataContainer::const_iterator end) const
{
  scatterData->clear();

  const QCPRange range = mValueAxis->range();
  bool reversed = mValueAxis->rangeReversed();
  const double clipMargin = range.size()*0.05;
  const double upperClipValue = range.upper + (reversed ? 0 : clipMargin); // clip slightly outside of actual range to avoid scatter size to peek into visible circle
  const double lowerClipValue = range.lower - (reversed ? clipMargin : 0); // clip slightly outside of actual range to avoid scatter size to peek into visible circle
  SkyGraphDataContainer::const_iterator it = begin;
  while (it != end)
  {
    if (it->value > lowerClipValue && it->value < upperClipValue)
      scatterData->append(*it);
    ++it;
  }
}

SkyGraphData::SkyGraphData()
    :t(0),key(0),value(0)
{

}

SkyGraphData::SkyGraphData(double t, double key, double value)
    :t(t),key(key),value(value)
{
}
