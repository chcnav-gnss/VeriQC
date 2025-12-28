#include "SkyPlotDialog.h"
#include "ui_SkyPlotDialog.h"
#include "QCObjectManager.h"

SkyPlotDialog::SkyPlotDialog(int qcObjectId, QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::SkyPlotDialog),m_qcObjectId(qcObjectId)
{
    ui->setupUi(this);

    this->setMinimumSize(600,500);

    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(qcObjectId);
    this->setWindowTitle(QString("%1 : %2").arg(tr("Sky Plot")).arg(qcObject->getName()));

    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor("#FFFFFF"));
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    m_plot = new SkyPlot(qcObjectId,this);
    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->setContentsMargins(0,0,0,0);
    plotLayout->addWidget(m_plot);
    ui->plotWidget->setLayout(plotLayout);

    ui->colorBar->setColorGradient(m_plot->getColorGradient());

    connect(ui->gpsCheckBox,&QCheckBox::clicked,this,&SkyPlotDialog::onGPSCheckBoxClicked);
    connect(ui->bdsCheckBox,&QCheckBox::clicked,this,&SkyPlotDialog::onBDSCheckBoxClicked);
    connect(ui->gloCheckBox,&QCheckBox::clicked,this,&SkyPlotDialog::onGLOCheckBoxClicked);
    connect(ui->galCheckBox,&QCheckBox::clicked,this,&SkyPlotDialog::onGALCheckBoxClicked);
    connect(ui->qzssCheckBox,&QCheckBox::clicked,this,&SkyPlotDialog::onQZSSCheckBoxClicked);

    this->updateSatCountInfo();
}

SkyPlotDialog::~SkyPlotDialog()
{
    delete ui;
}

void SkyPlotDialog::onGPSCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysGPS,checked);
}

void SkyPlotDialog::onBDSCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysBDS,checked);
}

void SkyPlotDialog::onGLOCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysGLO,checked);
}

void SkyPlotDialog::onGALCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysGAL,checked);
}

void SkyPlotDialog::onQZSSCheckBoxClicked(bool checked)
{
    m_plot->setSysVisible(QCResultManager::QCSysQZSS,checked);
}

void SkyPlotDialog::showEvent(QShowEvent *)
{
    ui->colorBar->setMinimumHeight(ui->sysWidget->height());
    QTimer::singleShot(500,m_plot,&SkyPlot::updatePlot);
}

void SkyPlotDialog::updateSatCountInfo()
{
    ui->bdsCountLabel->setText("0");
    ui->gpsCountLabel->setText("0");
    ui->gloCountLabel->setText("0");
    ui->galCountLabel->setText("0");
    ui->qzssCountLabel->setText("0");
    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(m_qcObjectId);
    if(!qcObject)
    {
        return;
    }
    ui->bdsCountLabel->setText(QString::number(qcObject->getQCResultManager()->getValidSysSatPrns(QCResultManager::QCSysBDS).size()));
    ui->gpsCountLabel->setText(QString::number(qcObject->getQCResultManager()->getValidSysSatPrns(QCResultManager::QCSysGPS).size()));
    ui->gloCountLabel->setText(QString::number(qcObject->getQCResultManager()->getValidSysSatPrns(QCResultManager::QCSysGLO).size()));
    ui->galCountLabel->setText(QString::number(qcObject->getQCResultManager()->getValidSysSatPrns(QCResultManager::QCSysGAL).size()));
    ui->qzssCountLabel->setText(QString::number(qcObject->getQCResultManager()->getValidSysSatPrns(QCResultManager::QCSysQZSS).size()));
}
