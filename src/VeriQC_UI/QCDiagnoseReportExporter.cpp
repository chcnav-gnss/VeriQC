/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Diagnostic Report Exporter Module
 * *-
 * @file QCDiagnoseReportExporter.cpp
 * @author CHC
 * @date 2026-06-15
 * @brief Implements HTML rendering for diagnostic summaries.
 *
 **************************************************************************/

#include "QCDiagnoseReportExporter.h"
#include "QCObject.h"
#include "QCObjectManager.h"
#include "QCResultManager.h"
#include "QCDiagnoseConstants.h"
#include "QCDiagnoseData.h"
#include "QCDiagnoseEngine.h"
#include "QCDiagnoseTextBuilder.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace
{

/**
 * @brief Return the accent hex color for a diagnostic level.
 *
 * L1 = emerald (excellent), L2 = amber (good),
 * L3 = orange (attention), L4 = red (poor).
 */
const char *levelAccent(int level)
{
    switch (level)
    {
        case 1:  return "#16a34a";
        case 2:  return "#eab308";
        case 3:  return "#f97316";
        default: return "#dc2626";
    }
}

/**
 * @brief Return the soft-tint hex color used as a card background wash.
 */
const char *levelSoftTint(int level)
{
    switch (level)
    {
        case 1:  return "#dcfce7";
        case 2:  return "#fef9c3";
        case 3:  return "#ffedd5";
        default: return "#fee2e2";
    }
}

/**
 * @brief Return the localized verdict word for a level.
 */
QString levelName(int level)
{
    switch (level)
    {
        case 1:  return QObject::tr("Excellent");
        case 2:  return QObject::tr("Good");
        case 3:  return QObject::tr("Attention");
        default: return QObject::tr("Poor");
    }
}

/**
 * @brief Return the CSS class suffix for a level (e.g. "lvl-1").
 *
 * Clamps to the supported range so an out-of-range value cannot
 * produce a dangling class name.
 */
QString levelCssClass(int level)
{
    return QString::fromLatin1("lvl-%1").arg(qBound(1, level, 4));
}

/**
 * @brief HTML-escape a plain string for safe embedding.
 */
QString htmlEscape(const QString &s)
{
    QString out = s;
    out.replace('&',  "&amp;");
    out.replace('<',  "&lt;");
    out.replace('>',  "&gt;");
    out.replace('"',  "&quot;");
    out.replace('\'', "&#39;");
    return out;
}

/**
 * @brief Format a fraction in [0, 1] as a percentage string.
 *
 * Negative values (sentinel "no data") render as "N/A".
 */
QString formatPercent(double frac, int prec = 1)
{
    if (frac < 0.0)
    {
        return QStringLiteral("N/A");
    }
    return QString::number(frac * 100.0, 'f', prec) + QChar('%');
}

/**
 * @brief Return inline SVG markup for a named icon.
 *
 * All icons are 24x24, stroke=currentColor, fill=none so the CSS
 * --accent / color cascade drives their hue.
 */
QString svgIcon(const QString &name)
{
    if (name == QLatin1String("slip"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<path d=\"M3 12a9 9 0 0 1 18 0\"/><path d=\"M3 12a9 9 0 0 0 18 0\"/>"
            "<path d=\"M12 7l-2 4h3l-2 4\"/></svg>");
    }
    if (name == QLatin1String("snr"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<path d=\"M5 21l3-3\"/><path d=\"M8 18l8-8\"/><path d=\"M16 10l3-3\"/>"
            "<path d=\"M19 7a3 3 0 0 0-3-3\"/><path d=\"M22 7a6 6 0 0 0-6-6\"/></svg>");
    }
    if (name == QLatin1String("mp"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<path d=\"M3 11h13\"/><path d=\"M13 7l4 4-4 4\"/>"
            "<path d=\"M3 19l8-5\" stroke-dasharray=\"2 2\"/></svg>");
    }
    if (name == QLatin1String("avail"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<circle cx=\"12\" cy=\"12\" r=\"9\"/><path d=\"M8 12l3 3l5-6\"/></svg>");
    }
    if (name == QLatin1String("verdict-good"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<path d=\"M12 2l8 4v6c0 5-3.5 8-8 10-4.5-2-8-5-8-10V6z\"/>"
            "<path d=\"M8 12l3 3l5-6\"/></svg>");
    }
    if (name == QLatin1String("verdict-watch"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<path d=\"M12 2l8 4v6c0 5-3.5 8-8 10-4.5-2-8-5-8-10V6z\"/>"
            "<path d=\"M12 8v5\"/><circle cx=\"12\" cy=\"16\" r=\".6\" "
            "fill=\"currentColor\" stroke=\"none\"/></svg>");
    }
    if (name == QLatin1String("verdict-poor"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<path d=\"M12 2l8 4v6c0 5-3.5 8-8 10-4.5-2-8-5-8-10V6z\"/>"
            "<path d=\"M9 9l6 6\"/><path d=\"M15 9l-6 6\"/></svg>");
    }
    if (name == QLatin1String("chevron"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<path d=\"M6 9l6 6l6-6\"/></svg>");
    }
    if (name == QLatin1String("top"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<path d=\"M12 19V5\"/><path d=\"M5 12l7-7l7 7\"/></svg>");
    }
    if (name == QLatin1String("brand"))
    {
        return QStringLiteral(
            "<svg viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" "
            "stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">"
            "<path d=\"M12 2l8 4v6c0 5-3.5 8-8 10-4.5-2-8-5-8-10V6z\"/>"
            "<path d=\"M8 12l3 3l5-6\"/></svg>");
    }
    return QString();
}

/**
 * @brief Pick the verdict-icon name for an overall level.
 */
QString verdictIconName(int overall)
{
    if (overall <= 1)
    {
        return QStringLiteral("verdict-good");
    }
    if (overall <= 2)
    {
        return QStringLiteral("verdict-watch");
    }
    return QStringLiteral("verdict-poor");
}

/**
 * @brief Compute the overall level as the worst of the four dimensions.
 *
 * Multipath tops out at Level 3; the remaining three dimensions cover
 * the full 1..4 range.
 */
int overallLevel(const QCDiag::DiagnosticResult &r)
{
    int worst = static_cast<int>(r.slipLevel);
    worst = qMax(worst, static_cast<int>(r.snrLevel));
    worst = qMax(worst, static_cast<int>(r.mpLevel));
    worst = qMax(worst, static_cast<int>(r.availLevel));
    return worst;
}

/**
 * @brief Build the multi-segment progress bar for a level.
 *
 * Renders @p maxLevel segments; the first @p level segments are lit.
 */
QString levelProgressHtml(int level, int maxLevel)
{
    QString html;
    QTextStream ts(&html);
    ts << "<div class=\"lvl-bar\">";
    for (int i = 1; i <= maxLevel; ++i)
    {
        ts << "<span class=\"seg" << (i <= level ? " on" : "") << "\"></span>";
    }
    ts << "</div>";
    ts.flush();
    return html;
}

/**
 * @brief Build an HTML <span> badge for a level number.
 *
 * Color is driven by the .lvl-N class so all theming stays in CSS.
 */
QString levelBadge(int level)
{
    return QString("<span class=\"badge %1\">L%2 %3 %4</span>")
        .arg(levelCssClass(level))
        .arg(level)
        .arg(QChar(0xB7)) // middle dot separator
        .arg(htmlEscape(levelName(level)));
}

/**
 * @brief Build the inline CSS used by the report.
 *
 * Single self-contained stylesheet: custom properties per level,
 * responsive grid, sticky nav, accordion details, dark mode,
 * print-friendly rules.
 */
QString buildCss()
{
    return QStringLiteral(
        ":root{"
        "--bg:#f5f7fb;--card:#fff;--fg:#1e293b;--muted:#64748b;"
        "--border:#e2e8f0;--accent:#16a34a;--accent-soft:#dcfce7;"
        "--shadow:0 2px 8px rgba(15,23,42,.06);"
        "}"
        "[data-level=\"L1\"]{--accent:#16a34a;--accent-soft:#dcfce7;}"
        "[data-level=\"L2\"]{--accent:#eab308;--accent-soft:#fef9c3;}"
        "[data-level=\"L3\"]{--accent:#f97316;--accent-soft:#ffedd5;}"
        "[data-level=\"L4\"]{--accent:#dc2626;--accent-soft:#fee2e2;}"
        "*{box-sizing:border-box;}"
        "html{scroll-behavior:smooth;}"
        "body{margin:0;font-family:'Segoe UI',-apple-system,BlinkMacSystemFont,Arial,sans-serif;"
        "background:var(--bg);color:var(--fg);line-height:1.6;}"
        "a{color:inherit;text-decoration:none;}"
        "svg{width:24px;height:24px;vertical-align:middle;}"
        ".topnav{position:sticky;top:0;z-index:50;background:rgba(255,255,255,.85);"
        "backdrop-filter:blur(10px);border-bottom:1px solid var(--border);"
        "padding:12px clamp(16px,4vw,32px);display:flex;align-items:center;gap:24px;}"
        ".topnav .brand{display:inline-flex;align-items:center;gap:8px;"
        "font-weight:700;font-size:18px;color:var(--accent);}"
        ".topnav .anchors{display:flex;gap:8px;flex-wrap:wrap;margin-left:auto;}"
        ".topnav .anchors a{display:inline-flex;align-items:center;gap:6px;"
        "padding:6px 14px;border-radius:999px;background:var(--card);"
        "border:1px solid var(--border);font-size:13px;color:var(--muted);"
        "transition:all .2s ease;}"
        ".topnav .anchors a:hover{color:var(--accent);border-color:var(--accent);"
        "transform:translateY(-1px);}"
        ".container{max-width:1100px;margin:0 auto;padding:clamp(16px,4vw,32px);}"
        ".hero{background:linear-gradient(135deg,var(--accent),#1e293b);color:#fff;"
        "border-radius:20px;padding:clamp(24px,5vw,40px);"
        "display:grid;grid-template-columns:1fr auto;gap:24px;align-items:center;"
        "box-shadow:0 10px 30px rgba(15,23,42,.12);margin-bottom:24px;}"
        ".hero .eyebrow{font-size:clamp(1.5rem,3.5vw,2.1rem);"
        "font-weight:800;letter-spacing:.01em;line-height:1.1;"
        "margin:0 0 6px;}"
        ".hero .eyebrow::before{content:\"\";display:block;"
        "width:42px;height:3px;background:rgba(255,255,255,.6);"
        "border-radius:2px;margin-bottom:14px;}"
        ".hero h1{margin:0 0 14px;font-size:clamp(1rem,2.2vw,1.35rem);"
        "font-weight:500;opacity:.88;line-height:1.3;}"
        ".hero .meta-pills{display:flex;flex-wrap:wrap;gap:8px;}"
        ".hero .meta-pill{display:inline-flex;align-items:center;"
        "background:rgba(255,255,255,.18);border:1px solid rgba(255,255,255,.25);"
        "padding:4px 12px;border-radius:999px;font-size:13px;}"
        ".hero__verdict{display:flex;flex-direction:column;align-items:center;"
        "background:rgba(255,255,255,.15);border-radius:16px;padding:16px 24px;min-width:160px;}"
        ".hero__verdict svg{width:48px;height:48px;margin-bottom:8px;}"
        ".hero__verdict .label{font-size:12px;opacity:.85;"
        "text-transform:uppercase;letter-spacing:.08em;}"
        ".hero__verdict .word{font-size:clamp(1.25rem,3vw,1.75rem);"
        "font-weight:800;margin-top:4px;}"
        ".meter{background:var(--card);border:1px solid var(--border);"
        "border-radius:16px;padding:16px 20px;margin-bottom:24px;box-shadow:var(--shadow);}"
        ".meter .track{position:relative;height:8px;background:var(--border);"
        "border-radius:4px;overflow:hidden;}"
        ".meter .fill{position:absolute;top:0;left:0;height:100%;"
        "background:linear-gradient(90deg,#16a34a 0%,#eab308 33%,#f97316 66%,#dc2626 100%);"
        "border-radius:4px;transition:width .4s ease;}"
        ".meter .ticks{display:flex;justify-content:space-between;"
        "margin-top:6px;font-size:11px;color:var(--muted);}"
        ".kpi-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));"
        "gap:clamp(12px,2vw,20px);margin-bottom:24px;}"
        ".kpi-card{position:relative;display:flex;flex-direction:column;"
        "background:var(--card);border:1px solid var(--border);"
        "border-left:4px solid var(--accent);border-radius:16px;padding:20px;"
        "box-shadow:var(--shadow);transition:transform .2s ease,box-shadow .2s ease;}"
        ".kpi-card:hover{transform:translateY(-3px);box-shadow:0 12px 24px rgba(15,23,42,.12);}"
        ".kpi-card .icon{color:var(--accent);margin-bottom:8px;}"
        ".kpi-card h3{margin:0 0 12px;font-size:14px;color:var(--muted);"
        "font-weight:600;text-transform:uppercase;letter-spacing:.04em;}"
        ".kpi-card .level{display:flex;align-items:baseline;gap:8px;margin-bottom:10px;}"
        ".kpi-card .level .num{font-size:2rem;font-weight:800;color:var(--accent);line-height:1;}"
        ".kpi-card .level .name{font-size:13px;color:var(--muted);}"
        ".lvl-bar{display:flex;gap:4px;margin-bottom:12px;}"
        ".lvl-bar .seg{flex:1;height:6px;background:var(--border);border-radius:3px;}"
        ".lvl-bar .seg.on{background:var(--accent);}"
        ".kpi-metrics{list-style:none;padding:0;margin:0 0 12px;font-size:13px;color:var(--fg);}"
        ".kpi-metrics li{padding:2px 0;}"
        ".kpi-metrics b{color:var(--accent);font-weight:700;}"
        ".kpi-link{margin-top:auto;padding-top:8px;font-size:12px;"
        "color:var(--accent);font-weight:600;}"
        ".kpi-link:hover{text-decoration:underline;}"
        ".detail-stack{display:flex;flex-direction:column;gap:16px;margin-bottom:24px;}"
        "details.detail-card{background:var(--card);border:1px solid var(--border);"
        "border-left:4px solid var(--accent);border-radius:16px;box-shadow:var(--shadow);"
        "overflow:hidden;scroll-margin-top:80px;}"
        "details.detail-card>summary{list-style:none;cursor:pointer;"
        "display:flex;align-items:center;gap:12px;padding:18px 20px;"
        "background:linear-gradient(90deg,var(--accent-soft),transparent);}"
        "details.detail-card>summary::-webkit-details-marker{display:none;}"
        "details.detail-card .icon{color:var(--accent);}"
        "details.detail-card .title{font-size:16px;font-weight:700;flex:1;}"
        "details.detail-card .chevron{color:var(--muted);transition:transform .2s ease;}"
        "details.detail-card[open] .chevron{transform:rotate(180deg);}"
        "details.detail-card .body{padding:16px 20px 20px;border-top:1px solid var(--border);}"
        "details.detail-card .body p{margin:8px 0;font-size:14px;line-height:1.7;}"
        "details.detail-card:target{outline:2px solid var(--accent);outline-offset:2px;}"
        ".badge{display:inline-block;color:#fff;padding:4px 12px;border-radius:999px;"
        "font-size:12px;font-weight:700;}"
        ".badge.lvl-1{background:#16a34a;}"
        ".badge.lvl-2{background:#ca8a04;}"
        ".badge.lvl-3{background:#ea580c;}"
        ".badge.lvl-4{background:#b91c1c;}"
        ".report-footer{display:flex;justify-content:space-between;flex-wrap:wrap;gap:8px;"
        "padding:20px 0;border-top:1px solid var(--border);font-size:12px;color:var(--muted);}"
        ".back-to-top{position:fixed;bottom:24px;right:24px;z-index:60;width:44px;height:44px;"
        "display:inline-flex;align-items:center;justify-content:center;background:var(--accent);"
        "color:#fff;border-radius:50%;box-shadow:0 6px 16px rgba(15,23,42,.2);"
        "transition:transform .2s ease,box-shadow .2s ease;}"
        ".back-to-top:hover{transform:translateY(-2px);box-shadow:0 10px 20px rgba(15,23,42,.3);}"
        ".back-to-top svg{width:20px;height:20px;}"
        "@keyframes fade-in{from{opacity:0;transform:translateY(8px);}"
        "to{opacity:1;transform:translateY(0);}}"
        ".fade-in{animation:fade-in .4s ease both;}"
        "@media (max-width:640px){"
        ".hero{grid-template-columns:1fr;}"
        ".hero__verdict{flex-direction:row;justify-content:center;}"
        "}"
        "@media (prefers-color-scheme: dark){"
        ":root{--bg:#0f172a;--card:#1e293b;--fg:#e2e8f0;--muted:#94a3b8;"
        "--border:#334155;--shadow:0 2px 8px rgba(0,0,0,.3);}"
        ".topnav{background:rgba(15,23,42,.85);}"
        "[data-level=\"L1\"]{--accent-soft:rgba(22,163,74,.18);}"
        "[data-level=\"L2\"]{--accent-soft:rgba(234,179,8,.18);}"
        "[data-level=\"L3\"]{--accent-soft:rgba(249,115,22,.18);}"
        "[data-level=\"L4\"]{--accent-soft:rgba(220,38,38,.18);}"
        "}"
        "@media print{"
        ".topnav,.back-to-top,.kpi-link{display:none !important;}"
        "body{background:#fff;}"
        ".hero{color:#000;background:#fff;border:2px solid #000;box-shadow:none;}"
        "details.detail-card,.kpi-card,.meter{box-shadow:none;break-inside:avoid;}"
        "details.detail-card>summary{background:none;}"
        "*{color-adjust:exact;-webkit-print-color-adjust:exact;print-color-adjust:exact;}"
        "}"
    );
}

/**
 * @brief Format 1-2 KPI metric lines for the cycle-slip card.
 *
 * Skips OSR when sentinel; the first non-empty line becomes the
 * headline metric.
 */
QStringList slipKeyMetrics(const QCDiag::DiagnosticInput &in)
{
    // Per-system slip ratio (OSR = detect/slip). Systems without slips
    // (sentinel kOSRInvalid) are skipped, so SBAS is usually absent.
    QStringList out;
    QStringList cells;
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        const double osr = in.slip.sysOSR[sys];
        if (osr >= QCDiagConst::kOSRInvalid)
        {
            continue;
        }
        cells << QString("<b>%1 %2</b>")
            .arg(QCDiag::QCDiagnoseTextBuilder::systemShortName(sys))
            .arg(QString::number(osr, 'f', 0));
    }
    if (cells.isEmpty())
    {
        cells << QObject::tr("No slip");
    }
    out << QString("%1: %2")
        .arg(QObject::tr("Per-system slip ratio"), cells.join(", "));
    return out;
}

/**
 * @brief Format KPI metric lines for the SNR card.
 */
QStringList snrKeyMetrics(const QCDiag::DiagnosticInput &in)
{
    QStringList out;
    out << QString("%1: <b>%2</b>")
        .arg(QObject::tr("Sat ratio with SNR < 35 dB"), formatPercent(in.snr.lowCN0SatRatio));
    out << QString("%1: <b>%2</b>")
        .arg(QObject::tr("SNR sharp-drop periods"))
        .arg(in.snr.alertDropWindowCount);
    return out;
}

/**
 * @brief Format KPI metric lines for the multipath card.
 *
 * Uses the cross-frequency averaged ratio for the <0.3m group; the
 * -1.0 sentinel ("no observations") is rendered as N/A.
 */
QStringList mpKeyMetrics(const QCDiag::DiagnosticInput &in)
{
    double ratioLt03 = -1.0;
    for (int f = 0; f < QC_NUM_FREQ; ++f)
    {
        const float v = in.mp.groupRatioAve[f][QCDiagConst::kMpGroupIdx03];
        if (v >= 0.0f && (ratioLt03 < 0.0 || v < ratioLt03))
        {
            ratioLt03 = v;
        }
    }
    QStringList out;
    out << QString("%1: <b>%2</b>")
        .arg(QObject::tr("Pseudorange MP < 0.3m ratio"), formatPercent(ratioLt03));
    return out;
}

/**
 * @brief Format KPI metric lines for the availability card.
 */
QStringList availKeyMetrics(const QCDiag::DiagnosticInput &in)
{
    QStringList out;
    out << QString("%1: <b>%2</b>")
        .arg(QObject::tr("Use Rate (total)"), formatPercent(in.avail.useRateTotal));
    out << QString("%1: <b>%2</b>")
        .arg(QObject::tr("Overall observation break count (breaks > 1 min)"))
        .arg(in.avail.obsBreakCount);

    // FR-A3: surface systems observed but with zero use rate (e.g. BDS with
    // ephemeris issues) on the KPI card so the gap is visible without
    // expanding details. Shares the anomaly predicate with the FR-09 detail
    // remark via anomalousSystemNames() (single source of truth).
    const QStringList anomalousSys =
        QCDiag::QCDiagnoseTextBuilder::anomalousSystemNames(in.avail);
    if (!anomalousSys.isEmpty())
    {
        out << QString("%1: <b>%2</b>")
            .arg(QObject::tr("Anomalous system(s)"))
            .arg(anomalousSys.join(", "));
    }
    return out;
}

/**
 * @brief One anchor entry for the sticky top navigation bar.
 */
struct NavAnchor
{
    QString id;        /**< Anchor target id (without leading '#'). */
    QString iconName;  /**< Inline-SVG icon name from svgIcon(). */
    QString label;     /**< Localized anchor label. */
};

/**
 * @brief Build the sticky top navigation bar.
 *
 * Renders the brand glyph + name on the left and one anchor pill per
 * entry on the right.
 */
QString topNavHtml(const QVector<NavAnchor> &anchors)
{
    QString html;
    QTextStream ts(&html);
    ts << "<nav class=\"topnav\"><div class=\"brand\">"
       << svgIcon(QStringLiteral("brand"))
       << "<span>VeriQC</span></div><div class=\"anchors\">";
    for (const NavAnchor &a : anchors)
    {
        ts << "<a href=\"#" << a.id << "\">"
           << svgIcon(a.iconName)
           << "<span>" << htmlEscape(a.label) << "</span></a>";
    }
    ts << "</div></nav>";
    ts.flush();
    return html;
}

/**
 * @brief Build the hero header with the overall verdict block.
 */
QString heroHtml(const QString &objName,
                 const QString &period,
                 const QString &generated,
                 int overall)
{
    QString html;
    QTextStream ts(&html);
    ts << "<header class=\"hero fade-in\" data-level=\"L" << overall << "\">"
       << "<div>"
       << "<div class=\"eyebrow\">"
       << htmlEscape(QObject::tr("QC Diagnostic Report")) << "</div>"
       << "<h1>" << htmlEscape(objName) << "</h1>"
       << "<div class=\"meta-pills\">"
       << "<span class=\"meta-pill\">" << htmlEscape(QObject::tr("Period"))
       << ": " << htmlEscape(period) << "</span>"
       << "<span class=\"meta-pill\">" << htmlEscape(QObject::tr("Generated"))
       << ": " << htmlEscape(generated) << "</span>"
       << "</div></div>"
       << "<div class=\"hero__verdict\">"
       << svgIcon(verdictIconName(overall))
       << "<div class=\"label\">" << htmlEscape(QObject::tr("Overall Verdict")) << "</div>"
       << "<div class=\"word\">" << htmlEscape(levelName(overall)) << "</div>"
       << "</div></header>";
    ts.flush();
    return html;
}

/**
 * @brief Build the severity meter strip.
 *
 * The fill width maps overall level 1..4 to 12.5/37.5/62.5/87.5 percent
 * so the bar visually centers on the active tier.
 */
QString meterHtml(int overall)
{
    static const int kPct[5] = {0, 12, 37, 62, 87};
    const int pct = kPct[qBound(1, overall, 4)];
    QString html;
    QTextStream ts(&html);
    ts << "<section class=\"meter fade-in\"><div class=\"track\">"
       << "<div class=\"fill\" style=\"width:" << pct << "%\"></div></div>"
       << "<div class=\"ticks\"><span>L1</span><span>L2</span>"
       << "<span>L3</span><span>L4</span></div></section>";
    ts.flush();
    return html;
}

/**
 * @brief Build a single KPI card.
 *
 * @p metrics is a list of pre-formatted HTML fragments (allowed to
 * contain <b> tags); they are joined into a <ul> without escaping.
 */
QString kpiCardHtml(const QString &anchor,
                    const QString &iconName,
                    const QString &title,
                    int level,
                    int maxLevel,
                    const QStringList &metrics)
{
    QString html;
    QTextStream ts(&html);
    ts << "<article class=\"kpi-card fade-in\" data-level=\"L" << level << "\">"
       << "<div class=\"icon\">" << svgIcon(iconName) << "</div>"
       << "<h3>" << htmlEscape(title) << "</h3>"
       << "<div class=\"level\">"
       << "<span class=\"num\">L" << level << "</span>"
       << "<span class=\"name\">" << htmlEscape(levelName(level)) << "</span>"
       << "</div>"
       << levelProgressHtml(level, maxLevel);
    if (!metrics.isEmpty())
    {
        ts << "<ul class=\"kpi-metrics\">";
        for (const QString &m : metrics)
        {
            ts << "<li>" << m << "</li>";
        }
        ts << "</ul>";
    }
    ts << "<a class=\"kpi-link\" href=\"#" << anchor << "\">"
       << htmlEscape(QObject::tr("View details")) << ' ' << QChar(0x2192) // right arrow
       << "</a></article>";
    ts.flush();
    return html;
}

/**
 * @brief Build a single collapsible detail card.
 *
 * @p paragraphs are plain text; this helper HTML-escapes each entry
 * before wrapping it in <p> tags.
 */
QString detailCardHtml(const QString &id,
                       const QString &iconName,
                       const QString &title,
                       int level,
                       const QStringList &paragraphs)
{
    QString html;
    QTextStream ts(&html);
    ts << "<details class=\"detail-card fade-in\" id=\"" << id
       << "\" data-level=\"L" << level << "\" open>"
       << "<summary>"
       << "<span class=\"icon\">" << svgIcon(iconName) << "</span>"
       << "<span class=\"title\">" << htmlEscape(title) << "</span>"
       << levelBadge(level)
       << "<span class=\"chevron\">" << svgIcon(QStringLiteral("chevron")) << "</span>"
       << "</summary>"
       << "<div class=\"body\">";
    for (const QString &p : paragraphs)
    {
        ts << "<p>" << htmlEscape(p) << "</p>";
    }
    ts << "</div></details>";
    ts.flush();
    return html;
}

} // namespace

int QCDiagnoseReportExporter::exportToHtml(const QString &filePath,
                                           const QVector<int> &qcObjectIDs)
{
    if (filePath.isEmpty() || qcObjectIDs.isEmpty())
    {
        return 0;
    }

    int written = 0;
    if (qcObjectIDs.size() == 1)
    {
        if (writeOneHtml(filePath, qcObjectIDs.first()))
        {
            written = 1;
        }
    }
    else
    {
        QDir dir(filePath);
        if (!dir.exists())
        {
            dir.mkpath(".");
        }
        for (int id : qcObjectIDs)
        {
            QSharedPointer<QCObject> obj = QCObjectManager::getInstance()->getQCObjectByID(id);
            if (!obj)
            {
                continue;
            }
            if (obj->getQCStatus() == QCObject::QCResultStatusEnum::QCStatusNotExecuted)
            {
                continue;
            }
            const QString outPath = dir.absoluteFilePath(
                QString("%1_QCDiag.html").arg(QFileInfo(obj->getName()).completeBaseName()));
            if (writeOneHtml(outPath, id))
            {
                ++written;
            }
        }
    }
    return written;
}

bool QCDiagnoseReportExporter::writeOneHtml(const QString &outputPath, int qcObjectID)
{
    QSharedPointer<QCObject> obj = QCObjectManager::getInstance()->getQCObjectByID(qcObjectID);
    if (!obj)
    {
        return false;
    }
    if (obj->getQCStatus() == QCObject::QCResultStatusEnum::QCStatusNotExecuted)
    {
        return false;
    }

    const QString html = composeHtml(qcObjectID);
    if (html.isEmpty())
    {
        return false;
    }

    QFile f(outputPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        return false;
    }
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    ts.setGenerateByteOrderMark(true);
    ts << html;
    f.close();
    return true;
}

QString QCDiagnoseReportExporter::composeHtml(int qcObjectID)
{
    QSharedPointer<QCObject> obj = QCObjectManager::getInstance()->getQCObjectByID(qcObjectID);
    if (!obj)
    {
        return QString();
    }
    const QSharedPointer<QCResultManager> &mgr = obj->getQCResultManager();
    if (!mgr)
    {
        return QString();
    }

    const QC_RESULT_T &raw = mgr->getQCResult();
    QCDiag::DiagnosticInput input = QCDiag::buildDiagnosticInput(raw, *mgr, QCDiagConst::kObsFileIndex, obj->getQCConfigurator()->getQCConfig().noEphEnable());
    QCDiag::DiagnosticResult result;
    QCDiag::DiagnoseEngine::diagnose(input, result);

    const QDateTime startTime = mgr->getStartTime();
    const QDateTime endTime   = mgr->getEndTime();
    const QString timeFmt = "yyyy/MM/dd HH:mm:ss";
    const QString periodStr    = startTime.toString(timeFmt) +
                                 QStringLiteral(" ~ ") +
                                 endTime.toString(timeFmt);
    const QString generatedStr = QDateTime::currentDateTime().toString(timeFmt);

    // Localized section titles (reused by both the KPI cards and the
    // detail-card headings so the wording stays consistent with the
    // existing translation context).
    const QString slipTitle  = QCoreApplication::translate("QCDiagnoseTextBuilder", "Cycle Slip Diagnosis");
    const QString snrTitle   = QCoreApplication::translate("QCDiagnoseTextBuilder", "SNR Diagnosis");
    const QString mpTitle    = QCoreApplication::translate("QCDiagnoseTextBuilder", "Multipath Diagnosis");
    const QString availTitle = QCoreApplication::translate("QCDiagnoseTextBuilder", "Availability Diagnosis");

    const int slipL  = static_cast<int>(result.slipLevel);
    const int snrL   = static_cast<int>(result.snrLevel);
    const int mpL    = static_cast<int>(result.mpLevel);
    const int availL = static_cast<int>(result.availLevel);
    const int overall = overallLevel(result);

    QVector<NavAnchor> anchors = {
        { QStringLiteral("slip"),  QStringLiteral("slip"),  slipTitle  },
        { QStringLiteral("snr"),   QStringLiteral("snr"),   snrTitle   },
        { QStringLiteral("mp"),    QStringLiteral("mp"),    mpTitle    },
        { QStringLiteral("avail"), QStringLiteral("avail"), availTitle },
    };

    QString html;
    QTextStream ts(&html);
    ts << "<!DOCTYPE html>";
    ts << "<html lang=\"en\"><head>";
    ts << "<meta charset=\"utf-8\">";
    ts << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    ts << "<title>" << htmlEscape(obj->getName())
       << ' ' << QChar(0xB7) << ' '
       << htmlEscape(QObject::tr("VeriQC QC Diagnostic Report"))
       << "</title>";
    ts << "<style>" << buildCss() << "</style>";
    ts << "</head><body id=\"top\">";

    ts << topNavHtml(anchors);
    ts << "<main class=\"container\">";
    ts << heroHtml(obj->getName(), periodStr, generatedStr, overall);
    ts << meterHtml(overall);

    /** no-ephemeris mode banner: alert user that elevation-dependent outputs are omitted */
    if (input.noEphemeris)
    {
        ts << "<div style=\"background:#FFF3CD;color:#856404;padding:10px 16px;"
              "border-radius:6px;margin:12px 0;font-weight:600;\">"
           << htmlEscape(QObject::tr("No Ephemeris Mode: elevation-angle data is unavailable; "
                                     "remarks referencing elevation angles (low-elevation masking, "
                                     "high/low-elevation attribution) are omitted. Slip, SNR, "
                                     "multipath and availability conclusions remain based on "
                                     "observations."))
           << "</div>";
    }

    // KPI dashboard
    ts << "<section class=\"kpi-grid\">";
    ts << kpiCardHtml(anchors[0].id, anchors[0].iconName, slipTitle,
                      slipL, 4, slipKeyMetrics(input));
    ts << kpiCardHtml(anchors[1].id, anchors[1].iconName, snrTitle,
                      snrL, 4, snrKeyMetrics(input));
    ts << kpiCardHtml(anchors[2].id, anchors[2].iconName, mpTitle,
                      mpL, 3, mpKeyMetrics(input));
    ts << kpiCardHtml(anchors[3].id, anchors[3].iconName, availTitle,
                      availL, 4, availKeyMetrics(input));
    ts << "</section>";

    // Detail cards
    ts << "<section class=\"detail-stack\">";
    ts << detailCardHtml(anchors[0].id, anchors[0].iconName, slipTitle, slipL,
                         QCDiag::QCDiagnoseTextBuilder::buildSlipSection(input, result, *mgr));
    ts << detailCardHtml(anchors[1].id, anchors[1].iconName, snrTitle, snrL,
                         QCDiag::QCDiagnoseTextBuilder::buildSnrSection(input, result));
    ts << detailCardHtml(anchors[2].id, anchors[2].iconName, mpTitle, mpL,
                         QCDiag::QCDiagnoseTextBuilder::buildMpSection(input, result));
    ts << detailCardHtml(anchors[3].id, anchors[3].iconName, availTitle, availL,
                         QCDiag::QCDiagnoseTextBuilder::buildAvailabilitySection(input, result, *mgr));
    ts << "</section>";

    // Footer
    ts << "<footer class=\"report-footer\">"
       << "<div>VeriQC " << QChar(0xB7) << ' '
       << htmlEscape(QObject::tr("QC Diagnostic Summary")) << "</div>"
       << "<div>" << htmlEscape(QObject::tr("Generated")) << ": "
       << htmlEscape(generatedStr) << "</div>"
       << "</footer>";

    ts << "</main>";

    // Back-to-top anchor (pure CSS, no JS required)
    ts << "<a class=\"back-to-top\" href=\"#top\" aria-label=\""
       << htmlEscape(QObject::tr("Back to top")) << "\">"
       << svgIcon(QStringLiteral("top")) << "</a>";

    ts << "</body></html>";
    ts.flush();
    return html;
}
