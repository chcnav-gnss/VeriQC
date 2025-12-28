#include "StationAttributeInfoWidget.h"
#include "StationAttributeInfoWidget.h"
#include "ui_StationAttributeInfoWidget.h"
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

StationAttributeInfoWidget::StationAttributeInfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StationAttributeInfoWidget)
{
    ui->setupUi(this);

    // init sub widgets style
    this->setStyleSheet("QLabel {min-height:30px;}");
    this->initInfoWidget(ui->stationInfoWidget);
    this->initInfoWidget(ui->obsInfoWidget);
    this->initInfoWidget(ui->recevierInfoWidget);
    this->initInfoWidget(ui->antennaInfoWidget);

    ui->inputFormatComboBox->addItem(QCConfigurator::getFileDataFormatName(QCConfigurator::RawFileDataFormatEnum::RTCM2),QCConfigurator::RawFileDataFormatEnum::RTCM2);
    ui->inputFormatComboBox->addItem(QCConfigurator::getFileDataFormatName(QCConfigurator::RawFileDataFormatEnum::RTCM3),QCConfigurator::RawFileDataFormatEnum::RTCM3);
    ui->inputFormatComboBox->addItem(QCConfigurator::getFileDataFormatName(QCConfigurator::RawFileDataFormatEnum::RINEX),QCConfigurator::RawFileDataFormatEnum::RINEX);
    ui->inputFormatComboBox->addItem(QCConfigurator::getFileDataFormatName(QCConfigurator::RawFileDataFormatEnum::HRCX),QCConfigurator::RawFileDataFormatEnum::HRCX);
    ui->inputFormatComboBox->addItem(QCConfigurator::getFileDataFormatName(QCConfigurator::RawFileDataFormatEnum::RawMsgAscii),QCConfigurator::RawFileDataFormatEnum::RawMsgAscii);
    ui->inputFormatComboBox->addItem(QCConfigurator::getFileDataFormatName(QCConfigurator::RawFileDataFormatEnum::RawMsgBinary),QCConfigurator::RawFileDataFormatEnum::RawMsgBinary);

    connect(ui->stationInfoToolButton,&QToolButton::clicked,this,&StationAttributeInfoWidget::onStationInfoToolButtonClicked);
    connect(ui->obsInfoToolButton,&QToolButton::clicked,this,&StationAttributeInfoWidget::onObsInfoToolButtonClicked);
    connect(ui->recevierInfoToolButton,&QToolButton::clicked,this,&StationAttributeInfoWidget::onRecevierInfoToolButtonClicked);
    connect(ui->antennaInfoToolButton,&QToolButton::clicked,this,&StationAttributeInfoWidget::onAntennaInfoToolButtonClicked);
    connect(ui->inputFormatSwitchButton,&QPushButton::clicked,this,&StationAttributeInfoWidget::onSwitchInputFormat);

    setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0,0);
    shadowEffect->setBlurRadius(6);
    shadowEffect->setColor(QColor(0, 0, 0, 25));
    this->setGraphicsEffect(shadowEffect);
    this->setStyleSheet("margin:6px;");
    ui->frame->setStyleSheet("QFrame{background-color: white;}");

    this->hide();
}

StationAttributeInfoWidget::~StationAttributeInfoWidget()
{
    delete ui;
}

void StationAttributeInfoWidget::updateInfo(QSharedPointer<QCObject> qcObject)
{
    m_qcObject = qcObject;
    ui->inputFormatComboBox->setCurrentText(QCConfigurator::getFileDataFormatName(
                                                qcObject->getQCConfigurator()->getRawFileInfo().rawFileDataFormat));
    ui->stationNameLabel->setText(qcObject->getName());
    QCResultManager::StationAttributeInfo stationAttributeInfo = qcObject->getQCResultManager()->getStationAttributeInfo();
    ui->ecefXLabel->setText(QString::number(stationAttributeInfo.ecef[0],'f',3));
    ui->ecefYLabel->setText(QString::number(stationAttributeInfo.ecef[1],'f',3));
    ui->ecefZLabel->setText(QString::number(stationAttributeInfo.ecef[2],'f',3));
    ui->startObsTimeLabel->setText(stationAttributeInfo.startObsTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->endObsTimeLabel->setText(stationAttributeInfo.endObsTime.toString("yyyy-MM-dd HH:mm:ss"));
    ui->intervalLabel->setText(QString::number(stationAttributeInfo.interval,'f',3));
    ui->epochNumberLabel->setText(QString::number(stationAttributeInfo.EpochCount));
    ui->obsDataNumberLabel->setText("Unknown"); //@TODO
    ui->recevierManufacturerLabel->setText(stationAttributeInfo.receiverManufacturer);
    ui->receiverTypeLabel->setText(stationAttributeInfo.receiverType);
    ui->recevierVersionLabel->setText(stationAttributeInfo.receiverVersion);
    ui->recevierSNLabel->setText(stationAttributeInfo.receiverSN);
    ui->measuredAntennaHeightLabel->setText("Unknown"); //@TODO
    ui->measurementMethodLabel->setText("Unknown"); //@TODO
    ui->antennaManufacturerLabel->setText(stationAttributeInfo.antennaManufacturer);
    ui->antennaTypeLabel->setText(stationAttributeInfo.antennaType);
    ui->typeAntennaEINEXLabel->setText("Unknown"); //@TODO
    ui->antennaSNLabel->setText(stationAttributeInfo.antennaType);
    ui->antHConsistCompLabel->setText(QString::number(stationAttributeInfo.AntDeltaH,'f',3));
}

void StationAttributeInfoWidget::hideWidget()
{
    QRect parentRect = parentWidget()->rect();
    QPoint endPos(parentRect.right() + width(), parentRect.top());

    QPropertyAnimation *animation = new QPropertyAnimation(this,"pos");
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->stop();
    animation->setStartValue(pos());
    animation->setEndValue(endPos);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    connect(animation, &QPropertyAnimation::finished, this, &QWidget::hide);
    m_qcObject = nullptr;
}

void StationAttributeInfoWidget::retranslateUi()
{
    ui->retranslateUi(this);
}

void StationAttributeInfoWidget::initInfoWidget(QWidget *widget)
{
    widget->setStyleSheet("border: 1px solid #cccccc;");
    widget->layout()->setContentsMargins(1,1,1,1);
    QGridLayout *gridLayout = qobject_cast<QGridLayout *>(widget->layout());
    if(gridLayout)
    {
        gridLayout->setHorizontalSpacing(0);
        gridLayout->setVerticalSpacing(0);
    }

}

void StationAttributeInfoWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    QRect parentRect = parentWidget()->rect();
    QPoint startPos(parentRect.right() + width(), parentRect.top());
    QPoint endPos(parentRect.right() - width(), parentRect.top());

    QPropertyAnimation *animation = new QPropertyAnimation(this,"pos");
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->stop();
    animation->setStartValue(startPos);
    animation->setEndValue(endPos);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    this->resize(this->width(),parentRect.height());
}

void StationAttributeInfoWidget::onStationInfoToolButtonClicked(bool checked)
{
    ui->stationInfoWidget->setVisible(checked);
    ui->stationInfoToolButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
}

void StationAttributeInfoWidget::onObsInfoToolButtonClicked(bool checked)
{
    ui->obsInfoWidget->setVisible(checked);
    ui->obsInfoToolButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
}

void StationAttributeInfoWidget::onRecevierInfoToolButtonClicked(bool checked)
{
    ui->recevierInfoWidget->setVisible(checked);
    ui->recevierInfoToolButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
}

void StationAttributeInfoWidget::onAntennaInfoToolButtonClicked(bool checked)
{
    ui->antennaInfoWidget->setVisible(checked);
    ui->antennaInfoToolButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
}

void StationAttributeInfoWidget::onSwitchInputFormat()
{
    QString currentFormt = QCConfigurator::getFileDataFormatName(m_qcObject->getQCConfigurator()->getRawFileInfo().rawFileDataFormat);
    if(currentFormt != ui->inputFormatComboBox->currentText())
    {
        m_qcObject->changeInputFormat(ui->inputFormatComboBox->currentData().value<QCConfigurator::RawFileDataFormatEnum>());
        emit inputFormatChanged(m_qcObject->getID());
    }
}
