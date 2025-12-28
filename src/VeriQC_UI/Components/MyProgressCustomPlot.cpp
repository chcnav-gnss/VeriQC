#include "MyProgressCustomPlot.h"
#include "MyWaterProgressBar.h"
#include "../TimePlotGraph.h"

MyProgressCustomPlot::MyProgressCustomPlot(QWidget *parent)
    :QCustomPlot(parent),QRunnable()
{
    this->setAutoDelete(false);
    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    connect(m_hoverTimer, &QTimer::timeout, this, &MyProgressCustomPlot::onMouseHover);
}

void MyProgressCustomPlot::execProgress()
{
    QDialog progressDialog(this);
    QTimer delayTimer;
    delayTimer.setInterval(500);
    delayTimer.setSingleShot(true);
    MyWaterProgressBar *progressBar = new MyWaterProgressBar(&progressDialog);
    progressBar->setFixedSize(200,200);
    QVBoxLayout *dialogLayout = new QVBoxLayout();
    dialogLayout->addWidget(progressBar);
    progressDialog.setLayout(dialogLayout);

    progressDialog.setWindowFlag(Qt::FramelessWindowHint);
    progressDialog.setAttribute(Qt::WA_TranslucentBackground);
    connect(this,&MyProgressCustomPlot::threadFinished,&delayTimer, QOverload<>::of(&QTimer::start));
    connect(&delayTimer,&QTimer::timeout,&progressDialog,&QProgressDialog::accept);
    connect(this,&MyProgressCustomPlot::threadProgressRateChanged,progressBar,&MyWaterProgressBar::setValue);
    QThreadPool::globalInstance()->start(this);
    /** show duration */
    QTimer showDelayTimer;
    QEventLoop loop;
    showDelayTimer.setSingleShot(true);
    showDelayTimer.callOnTimeout(&loop,&QEventLoop::quit);
    showDelayTimer.start(500);
    loop.exec();
    if (progressBar->value() < progressBar->maximum())
    {
        progressDialog.exec();
    }
    else
    {
        delayTimer.stop();
    }
}

void MyProgressCustomPlot::run()
{
    emit threadStarted();
    emit threadProgressRateChanged(0);
    this->doRun();
    emit threadProgressRateChanged(100);
    emit threadFinished();
}

void MyProgressCustomPlot::onMouseHover()
{
    QList<QVariant> details;
    QList<QCPLayerable*> candidates = layerableListAt(m_lastMousePos, false, &details);
    if (!candidates.isEmpty())
    {
        TimePlotGraph* ap = qobject_cast<TimePlotGraph*>(candidates.first());
        if (ap)
        {
            int dataIndex = 0;
            if (!details.first().value<QCPDataSelection>().isEmpty())
            {
                dataIndex = details.first().value<QCPDataSelection>().dataRange().begin();
            }
            if (0 == dataIndex)
            {
                return;
            }
            const QCPCurveData* hoverData = ap->data()->at(dataIndex);
            this->onPointHover(hoverData, m_lastMousePos);
        }
    }
}

void MyProgressCustomPlot::leaveEvent(QEvent*)
{
    m_hoverTimer->stop();
}

void MyProgressCustomPlot::mouseMoveEvent(QMouseEvent* event)
{
    m_lastMousePos = event->pos();
    m_hoverTimer->start(500);   /**< if mouse keep hovering one point more than 500ms, then show point info */
    QCustomPlot::mouseMoveEvent(event);
}

void MyProgressCustomPlot::onPointHover(const QCPCurveData* hoverPoint, const QPoint& mousePos)
{
    double y = hoverPoint->value;
    QToolTip::showText(this->mapToGlobal(mousePos), QString::number(y, 'f', 3), this);
}
