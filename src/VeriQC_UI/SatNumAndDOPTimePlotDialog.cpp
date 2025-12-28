#include "SatNumAndDOPTimePlotDialog.h"
#include "ui_SatNumAndDOPTimePlotDialog.h"
#include "QCObjectManager.h"

SatNumAndDOPTimePlotDialog::SatNumAndDOPTimePlotDialog(int qcObjectId, QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::SatNumAndDOPTimePlotDialog)
{
    ui->setupUi(this);

    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor("#FFFFFF"));
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(qcObjectId);
    this->setWindowTitle(QString("%1 : %2").arg(tr("SatNum/DOP Plot")).arg(qcObject->getName()));

    m_plot = new SatNumAndDOPTimePlot(qcObjectId,this);
    QVBoxLayout *plotLayout = new QVBoxLayout();
    plotLayout->addWidget(m_plot);
    plotLayout->setContentsMargins(0,0,0,0);
    ui->plotWidget->setLayout(plotLayout);

    ui->satNumCheckBox->setStyleSheet(QString("QCheckBox::indicator:unchecked{"
                                "background-color: \"%1\";"
                                "border-image: none;"
                            "}").arg(m_plot->getSatNumGraphColor().name()));
    ui->satNumCheckBox->setCheckable(false);    // sat num check box as color legend

    ui->pDOPCheckBox->setStyleSheet(QString("QCheckBox::indicator:checked{"
                                "background-color: \"%1\";"
                                "border-image: url(:/icons/Resources/Icons/CheckedWhite.png)"
                            "}").arg(m_plot->getPDOPGraphColor().name()));
    ui->vDOPCheckBox->setStyleSheet(QString("QCheckBox::indicator:checked{"
                                "background-color: \"%1\";"
                                "border-image: url(:/icons/Resources/Icons/CheckedWhite.png)"
                            "}").arg(m_plot->getVDOPGraphColor().name()));
    ui->hDOPCheckBox->setStyleSheet(QString("QCheckBox::indicator:checked{"
                                "background-color: \"%1\";"
                                "border-image: url(:/icons/Resources/Icons/CheckedWhite.png)"
                            "}").arg(m_plot->getHDOPGraphColor().name()));
    ui->tDOPCheckBox->setStyleSheet(QString("QCheckBox::indicator:checked{"
                                "background-color: \"%1\";"
                                "border-image: url(:/icons/Resources/Icons/CheckedWhite.png)"
                            "}").arg(m_plot->getTDOPGraphColor().name()));

    connect(ui->pDOPCheckBox,&QCheckBox::clicked,m_plot,&SatNumAndDOPTimePlot::setPDOPGraphVisible);
    connect(ui->vDOPCheckBox,&QCheckBox::clicked,m_plot,&SatNumAndDOPTimePlot::setVDOPGraphVisible);
    connect(ui->hDOPCheckBox,&QCheckBox::clicked,m_plot,&SatNumAndDOPTimePlot::setHDOPGraphVisible);
    connect(ui->tDOPCheckBox,&QCheckBox::clicked,m_plot,&SatNumAndDOPTimePlot::setTDOPGraphVisible);

    QTimer::singleShot(0,m_plot,&SatNumAndDOPTimePlot::updatePlot);
}

SatNumAndDOPTimePlotDialog::~SatNumAndDOPTimePlotDialog()
{
    delete ui;
}
