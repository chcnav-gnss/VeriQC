#include "ObsTimePlotDialog.h"
#include <QTimer>
#include "ui_ObsTimePlotDialog.h"
#include "QCObjectManager.h"

ObsTimePlotDialog::ObsTimePlotDialog(int qcObjectId, QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::ObsTimePlotDialog)
{
    ui->setupUi(this);

    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor("#FFFFFF"));
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    m_plot = new ObsTimePlot(qcObjectId,ui->scrollAreaWidgetContents);
    QVBoxLayout *contentsWidgetLayout = new QVBoxLayout();
    contentsWidgetLayout->addWidget(m_plot);
    contentsWidgetLayout->setContentsMargins(0,0,0,0);
    ui->scrollAreaWidgetContents->setLayout(contentsWidgetLayout);

    ui->legendIconLabel_1->setPixmap(this->generateFreqLegendIcon(m_plot->getFreqColor(0)));
    ui->legendIconLabel_2->setPixmap(this->generateFreqLegendIcon(m_plot->getFreqColor(1)));
    ui->legendIconLabel_3->setPixmap(this->generateFreqLegendIcon(m_plot->getFreqColor(2)));
    ui->legendIconLabel_4->setPixmap(this->generateFreqLegendIcon(m_plot->getFreqColor(3)));
    ui->legendIconLabel_5->setPixmap(this->generateFreqLegendIcon(m_plot->getFreqColor(4)));
    ui->legendIconLabel_6->setPixmap(this->generateFreqLegendIcon(m_plot->getFreqColor(5)));
    ui->legendIconLabel_7->setPixmap(this->generateFreqLegendIcon(m_plot->getFreqColor(6)));
    ui->cycleJumpLegendIconLabel->setPixmap(this->generateCycleJumpLegendIcon());

    ui->legnedLabel_1->setText(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP1));
    ui->legnedLabel_2->setText(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP2));
    ui->legnedLabel_3->setText(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP5));
    ui->legnedLabel_4->setText(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP6));
    ui->legnedLabel_5->setText(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP7));
    ui->legnedLabel_6->setText(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP8));
    ui->legnedLabel_7->setText(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexPlotExtend));

    connect(ui->gpsCheckBox,&QCheckBox::clicked,this,&ObsTimePlotDialog::onGPSCheckBoxClicked);
    connect(ui->bdsCheckBox,&QCheckBox::clicked,this,&ObsTimePlotDialog::onBDSCheckBoxClicked);
    connect(ui->gloCheckBox,&QCheckBox::clicked,this,&ObsTimePlotDialog::onGLOCheckBoxClicked);
    connect(ui->galCheckBox,&QCheckBox::clicked,this,&ObsTimePlotDialog::onGALCheckBoxClicked);
    connect(ui->qzssCheckBox,&QCheckBox::clicked,this,&ObsTimePlotDialog::onQZSSCheckBoxClicked);

    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(qcObjectId);

    this->setWindowTitle(QString("%1 : %2").arg(tr("Obs Time Plot")).arg(qcObject->getName()));
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

ObsTimePlotDialog::~ObsTimePlotDialog()
{
    delete ui;
}

void ObsTimePlotDialog::onGPSCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysGPS,checked);
    this->updatePlot();
}

void ObsTimePlotDialog::onGLOCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysGLO,checked);
    this->updatePlot();
}

void ObsTimePlotDialog::onQZSSCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysQZSS,checked);
    this->updatePlot();
}

void ObsTimePlotDialog::onGALCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysGAL,checked);
    this->updatePlot();
}

void ObsTimePlotDialog::onBDSCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysBDS,checked);
    this->updatePlot();
}

QPixmap ObsTimePlotDialog::generateFreqLegendIcon(QColor color)
{
    QPixmap pixmap(24, 4);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(0, 0, 24, 4, color);

    return pixmap;
}

QPixmap ObsTimePlotDialog::generateCycleJumpLegendIcon()
{
    QPixmap pixmap(8, 8);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setBrush(QColor("#FF3434"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 8, 8);

    return pixmap;
}

void ObsTimePlotDialog::showEvent(QShowEvent *)
{
    QTimer::singleShot(500,this,&ObsTimePlotDialog::updatePlot);
}

void ObsTimePlotDialog::updatePlot()
{
    m_plot->updatePlot();
}
