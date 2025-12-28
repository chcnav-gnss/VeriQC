#include "PlotTimeAxisTicker.h"
#include <QDateTime>

PlotTimeAxisTicker::PlotTimeAxisTicker()
{

}

void PlotTimeAxisTicker::generate(const QCPRange &range, const QLocale &locale, QChar formatChar, int precision, QVector<double> &ticks, QVector<double> *subTicks, QVector<QString> *tickLabels)
{
    // generate (major) ticks:
    double tickStep = getTickStep(range);
    if (mSameInterval)
    {
        if (tickStep > 100.f && tickStep <= 180.f)
        {
            tickStep = 180.f;
        }
        else if (tickStep > 2.f && tickStep <= 3.f)
        {
            tickStep = 3.f;
        }
    }
    tickStep = this->getClosestClockTickStep(tickStep);
    ticks = createTickVector(tickStep, range);
    trimTicks(range, ticks, true); // trim ticks to visible range plus one outer tick on each side (incase a subclass createTickVector creates more)

    // generate sub ticks between major ticks:
    if (subTicks)
    {
        if (!ticks.isEmpty())
        {
            *subTicks = createSubTickVector(getSubTickCount(tickStep), ticks);
            trimTicks(range, *subTicks, false);
        }
        else
            *subTicks = QVector<double>();
    }

    // finally trim also outliers (no further clipping happens in axis drawing):
    trimTicks(range, ticks, false);
    // generate labels for visible ticks if requested:
    if (tickLabels)
        *tickLabels = createLabelVector(ticks, locale, formatChar, precision,tickStep);
}

QVector<QString> PlotTimeAxisTicker::createLabelVector(const QVector<double> &ticks, const QLocale &locale, QChar formatChar, int precision, double tickStep)
{
    Q_UNUSED(locale)
    Q_UNUSED(formatChar)
    Q_UNUSED(precision)

    QVector<QString> result;
    result.reserve(ticks.size());
    QDateTime firstTick = QDateTime::fromMSecsSinceEpoch(ticks.first());
    QDateTime lastTick = QDateTime::fromMSecsSinceEpoch(ticks.last());
    bool isSameDay = firstTick.date() == lastTick.date();

    /** set time format */
    QString dateTimeFormat = "yyyy/MM/dd\nHH:mm:ss";
    if (tickStep > 23 * 60 * 60 * 1000)
    {
        dateTimeFormat = "yyyy/MM/dd";
    }
    else if (tickStep > 1000)
    {
        dateTimeFormat = isSameDay ? "HH:mm:ss" : "yyyy/MM/dd\nHH:mm:ss";
    }
    else
    {
        dateTimeFormat = "HH:mm:ss.z";
    }

    for(double tickCoord : ticks)
    {
        result.append(QDateTime::fromMSecsSinceEpoch(tickCoord).toString(dateTimeFormat));
    }

    return result;
}

double PlotTimeAxisTicker::getClosestClockTickStep(double tickStep)
{
    static const QVector<double> s_clockTickSteps = {
        100,                    // 100 ms
        200,                    // 200 ms
        500,                    // 500 ms
        1000,                   // 1 second
        2 * 1000,               // 2 second
        5 * 1000,               // 5 second
        15 * 1000,              // 15 seconds
        30 * 1000,              // 30 seconds
        60 * 1000,              // 1 minute
        2 * 60 * 1000,          // 2 minutes
        5 * 60 * 1000,          // 5 minutes
        15 * 60 * 1000,         // 15 minutes
        30 * 60 * 1000,         // 30 minutes
        60 * 60 * 1000,         // 1 hour
        3 * 60 * 60 * 1000,     // 3 hours
        6 * 60 * 60 * 1000,     // 6 hours
        12 * 60 * 60 * 1000,    // 12 hours
        24 * 60 * 60 * 1000,    // 1 day
    };
    if (tickStep < s_clockTickSteps.last())
    {
        return pickClosest(tickStep, s_clockTickSteps);
    }
    return static_cast<int>(tickStep / s_clockTickSteps.last()) * s_clockTickSteps.last();
}
