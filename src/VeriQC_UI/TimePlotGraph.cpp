#include "TimePlotGraph.h"

TimePlotGraph::TimePlotGraph(QCPAxis *keyAxis, QCPAxis *valueAxis)
    :QCPCurve(keyAxis,valueAxis)
{
    this->setLineStyle(TimePlotGraph::lsNone);
    this->setAntialiased(false);
    this->setAntialiasedScatters(false);
    this->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,2));
}

TimePlotGraph::~TimePlotGraph()
{

}

void TimePlotGraph::getDataScatters(QVector<QCPCurveData> *scatters, const QCPDataRange &dataRange, double scatterWidth)
{
    if (!scatters) return;
    scatters->clear();
    QCPAxis* keyAxis = mKeyAxis.data();
    QCPAxis* valueAxis = mValueAxis.data();
    if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }

    QCPCurveDataContainer::const_iterator begin = mDataContainer->constBegin();
    QCPCurveDataContainer::const_iterator end = mDataContainer->constEnd();
    mDataContainer->limitIteratorsToDataRange(begin, end, dataRange);
    if (begin == end)
        return;
    const int scatterModulo = mScatterSkip + 1;
    const bool doScatterSkip = mScatterSkip > 0;
    int endIndex = int(end - mDataContainer->constBegin());

    QCPRange keyRange = keyAxis->range();
    QCPRange valueRange = valueAxis->range();
    // extend range to include width of scatter symbols:
    keyRange.lower = keyAxis->pixelToCoord(keyAxis->coordToPixel(keyRange.lower) - scatterWidth * keyAxis->pixelOrientation());
    keyRange.upper = keyAxis->pixelToCoord(keyAxis->coordToPixel(keyRange.upper) + scatterWidth * keyAxis->pixelOrientation());
    valueRange.lower = valueAxis->pixelToCoord(valueAxis->coordToPixel(valueRange.lower) - scatterWidth * valueAxis->pixelOrientation());
    valueRange.upper = valueAxis->pixelToCoord(valueAxis->coordToPixel(valueRange.upper) + scatterWidth * valueAxis->pixelOrientation());

    QCPCurveDataContainer::const_iterator it = begin;
    int itIndex = int(begin - mDataContainer->constBegin());
    while (doScatterSkip && it != end && itIndex % scatterModulo != 0) // advance begin iterator to first non-skipped scatter
    {
        ++itIndex;
        ++it;
    }
    if (keyAxis->orientation() == Qt::Vertical)
    {
        while (it != end)
        {
            if (!qIsNaN(it->value) && keyRange.contains(it->key) && valueRange.contains(it->value))
                scatters->append(QCPCurveData(it->t, valueAxis->coordToPixel(it->value), keyAxis->coordToPixel(it->key)));

            // advance iterator to next (non-skipped) data point:
            if (!doScatterSkip)
                ++it;
            else
            {
                itIndex += scatterModulo;
                if (itIndex < endIndex) // make sure we didn't jump over end
                    it += scatterModulo;
                else
                {
                    it = end;
                    itIndex = endIndex;
                }
            }
        }
    }
    else
    {
        while (it != end)
        {
            if (!qIsNaN(it->value) && keyRange.contains(it->key) && valueRange.contains(it->value))
                scatters->append(QCPCurveData(it->t, keyAxis->coordToPixel(it->key), valueAxis->coordToPixel(it->value)));

            // advance iterator to next (non-skipped) data point:
            if (!doScatterSkip)
                ++it;
            else
            {
                itIndex += scatterModulo;
                if (itIndex < endIndex) // make sure we didn't jump over end
                    it += scatterModulo;
                else
                {
                    it = end;
                    itIndex = endIndex;
                }
            }
        }
    }
    graphPxielSampling(*scatters);
}

void TimePlotGraph::draw(QCPPainter *painter)
{
    if (mDataContainer->isEmpty()) return;

    // allocate line vector:
    QVector<QPointF> lines;
    QVector<QCPCurveData> scatters;

    // loop over and draw segments of unselected/selected data:
    QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
    getDataSegments(selectedSegments, unselectedSegments);
    allSegments << unselectedSegments << selectedSegments;
    for (int i = 0; i < allSegments.size(); ++i)
    {
        bool isSelectedSegment = i >= unselectedSegments.size();

        // fill with curve data:
        QPen finalCurvePen = mPen; // determine the final pen already here, because the line optimization depends on its stroke width
        if (isSelectedSegment && mSelectionDecorator)
            finalCurvePen = mSelectionDecorator->pen();

        QCPDataRange lineDataRange = isSelectedSegment ? allSegments.at(i) : allSegments.at(i).adjusted(-1, 1); // unselected segments extend lines to bordering selected data point (safe to exceed total data bounds in first/last segment, getCurveLines takes care)
        getCurveLines(&lines, lineDataRange, finalCurvePen.widthF());

        // check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA
        for (QCPCurveDataContainer::const_iterator it = mDataContainer->constBegin(); it != mDataContainer->constEnd(); ++it)
        {
            if (QCP::isInvalidData(it->t) ||
                QCP::isInvalidData(it->key, it->value))
                qDebug() << Q_FUNC_INFO << "Data point at" << it->key << "invalid." << "Plottable name:" << name();
    }
#endif

        // draw curve fill:
        applyFillAntialiasingHint(painter);
        if (isSelectedSegment && mSelectionDecorator)
            mSelectionDecorator->applyBrush(painter);
        else
            painter->setBrush(mBrush);
        painter->setPen(Qt::NoPen);
        if (painter->brush().style() != Qt::NoBrush && painter->brush().color().alpha() != 0)
            painter->drawPolygon(QPolygonF(lines));

        // draw curve line:
        if (mLineStyle != lsNone)
        {
            painter->setPen(finalCurvePen);
            painter->setBrush(Qt::NoBrush);
            drawCurveLine(painter, lines);
        }

        // draw scatters:
        QCPScatterStyle finalScatterStyle = mScatterStyle;
        if (isSelectedSegment && mSelectionDecorator)
            finalScatterStyle = mSelectionDecorator->getFinalScatterStyle(mScatterStyle);
        if (!finalScatterStyle.isNone())
        {
            getDataScatters(&scatters, allSegments.at(i), finalScatterStyle.size());
            drawGraphScatterPlot(painter, scatters, finalScatterStyle);
        }
}

    // draw other selection decoration that isn't just line/scatter pens and brushes:
    if (mSelectionDecorator)
        mSelectionDecorator->drawDecoration(painter, selection());
}

void TimePlotGraph::drawGraphScatterPlot(QCPPainter *painter, const QVector<QCPCurveData> &scatters, const QCPScatterStyle &style) const
{
    applyScattersAntialiasingHint(painter);
    style.applyTo(painter, mPen);
    painter->save();

    for(const QCPCurveData& scatter : scatters)
    {
        if (!qIsNaN(scatter.key) && !qIsNaN(scatter.value))
            style.drawShape(painter, scatter.key, scatter.value);
    }

    painter->restore();
}

void TimePlotGraph::graphPxielSampling(QVector<QCPCurveData> &scatters) const
{
    QCPAxis* keyAxis = mKeyAxis.data();
    QCPAxis* valueAxis = mValueAxis.data();
    if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }

    int left = keyAxis->coordToPixel(keyAxis->range().lower);
    int right = keyAxis->coordToPixel(keyAxis->range().upper);
    int top = valueAxis->coordToPixel(valueAxis->range().upper);
    int bottom = valueAxis->coordToPixel(valueAxis->range().lower);

    QVector<bool> pixelMap(abs((right - left + 1) * (bottom - top + 1)));
    QList<QCPCurveData> result;
    for (auto scatter : scatters)
    {
        int keyPixel = scatter.key;
        int valuePixel = scatter.value;
        if (keyPixel < left || keyPixel > right || valuePixel < top || valuePixel > bottom)
        {
            continue;
        }
        int pixelIndex = (keyPixel - left) + (valuePixel - top) * (right - left + 1);
        if (!pixelMap[pixelIndex])  /**< if this pixel has drawn some scatters, ignore other scatters in same pixel */
        {
            pixelMap[pixelIndex] = true;
            result.append(scatter);
        }
    }
    scatters = result.toVector();
}
