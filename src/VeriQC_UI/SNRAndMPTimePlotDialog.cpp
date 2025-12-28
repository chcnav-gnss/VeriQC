#include "SNRAndMPTimePlotDialog.h"
#include "QCObjectManager.h"
#include "ui_SNRAndMPTimePlotDialog.h"

SNRAndMPTimePlotDialog::SNRAndMPTimePlotDialog(int qcObjectId, QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::SNRAndMPTimePlotDialog)
{
    ui->setupUi(this);

    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor("#FFFFFF"));
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(qcObjectId);
    this->setWindowTitle(QString("%1 : %2").arg(tr("SNR/Multipath Plot")).arg(qcObject->getName()));

    m_plot = new SNRAndMPTimePlot(qcObjectId,this);
    QVBoxLayout *plotLayout = new QVBoxLayout();
    plotLayout->addWidget(m_plot);
    plotLayout->setContentsMargins(0,0,0,0);
    ui->plotWidget->setLayout(plotLayout);

    this->initSatComboBox(qcObject->getQCResultManager());

    ui->checkBoxBDS->setStyleSheet(QString("QCheckBox::indicator:unchecked{"
                                "background-color: \"%1\";"
                                "border-image: none;}").arg(m_plot->getSystemGraphColor(QCResultManager::QCSysBDS)));
    ui->checkBoxGPS->setStyleSheet(QString("QCheckBox::indicator:unchecked{"
                                           "background-color: \"%1\";"
                                           "border-image: none;}").arg(m_plot->getSystemGraphColor(QCResultManager::QCSysGPS)));
    ui->checkBoxGLO->setStyleSheet(QString("QCheckBox::indicator:unchecked{"
                                           "background-color: \"%1\";"
                                           "border-image: none;}").arg(m_plot->getSystemGraphColor(QCResultManager::QCSysGLO)));
    ui->checkBoxGAL->setStyleSheet(QString("QCheckBox::indicator:unchecked{"
                                           "background-color: \"%1\";"
                                           "border-image: none;}").arg(m_plot->getSystemGraphColor(QCResultManager::QCSysGAL)));
    ui->checkBoxQZSS->setStyleSheet(QString("QCheckBox::indicator:unchecked{"
                                            "background-color: \"%1\";"
                                            "border-image: none;}").arg(m_plot->getSystemGraphColor(QCResultManager::QCSysQZSS)));
    connect(ui->satComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&SNRAndMPTimePlotDialog::onSatChanged);
    connect(ui->freqComboBox,&QComboBox::currentTextChanged,this,&SNRAndMPTimePlotDialog::onFreqChanged);

    // sat combo box index = 0, freq combo box index = 0
    if(ui->satComboBox->count() > 0)
    {
        this->onSatChanged(0);
    }
}

SNRAndMPTimePlotDialog::~SNRAndMPTimePlotDialog()
{
    delete ui;
}

void SNRAndMPTimePlotDialog::initSatComboBox(const QSharedPointer<QCResultManager> &result)
{
    ui->satComboBox->clear();
    ui->satComboBox->addItem("All",QVariant::fromValue(SatComboBoxItemData{QCResultManager::QCSysComplex,0}));
    ui->satComboBox->addItem(QCResultManager::getSatName(QCResultManager::QCSysBDS,1).left(1),QVariant::fromValue(SatComboBoxItemData{QCResultManager::QCSysBDS,0}));   // all sats of one sys
    ui->satComboBox->addItem(QCResultManager::getSatName(QCResultManager::QCSysGPS,1).left(1),QVariant::fromValue(SatComboBoxItemData{QCResultManager::QCSysGPS,0}));   // all sats of one sys
    ui->satComboBox->addItem(QCResultManager::getSatName(QCResultManager::QCSysGLO,1).left(1),QVariant::fromValue(SatComboBoxItemData{QCResultManager::QCSysGLO,0}));   // all sats of one sys
    ui->satComboBox->addItem(QCResultManager::getSatName(QCResultManager::QCSysGAL,1).left(1),QVariant::fromValue(SatComboBoxItemData{QCResultManager::QCSysGAL,0}));   // all sats of one sys
    ui->satComboBox->addItem(QCResultManager::getSatName(QCResultManager::QCSysQZSS,1).left(1),QVariant::fromValue(SatComboBoxItemData{QCResultManager::QCSysQZSS,0}));   // all sats of one sys

    this->satComboBoxAddSysSats(result,QCResultManager::QCSysBDS);
    this->satComboBoxAddSysSats(result,QCResultManager::QCSysGPS);
    this->satComboBoxAddSysSats(result,QCResultManager::QCSysGLO);
    this->satComboBoxAddSysSats(result,QCResultManager::QCSysGAL);
    this->satComboBoxAddSysSats(result,QCResultManager::QCSysQZSS);
}

void SNRAndMPTimePlotDialog::satComboBoxAddSysSats(const QSharedPointer<QCResultManager> &result, QCResultManager::QCSysTypeEnum sys)
{
    QList<unsigned int> prns = result->getValidSysSatPrns(sys);
    for(unsigned int prn :prns)
    {
        ui->satComboBox->addItem(QCResultManager::getSatName(sys,prn),QVariant::fromValue(SatComboBoxItemData{sys,prn}));
    }
}

void SNRAndMPTimePlotDialog::onSatChanged(int index)
{
    disconnect(ui->freqComboBox,&QComboBox::currentTextChanged,this,&SNRAndMPTimePlotDialog::onFreqChanged);
    ui->freqComboBox->clear();
    if(index >= 0)
    {
        SatComboBoxItemData satData = ui->satComboBox->currentData().value<SatComboBoxItemData>();
        QCResultManager::QCSysTypeEnum sys = satData.sys;
        switch (sys)
        {
        case QCResultManager::QCSysGPS:
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GPS_L1),FREQ_INDEX_GPS_L1);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GPS_L2C),FREQ_INDEX_GPS_L2C);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GPS_L5),FREQ_INDEX_GPS_L5);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GPS_L1C),FREQ_INDEX_GPS_L1C);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GPS_L2P),FREQ_INDEX_GPS_L2P);
            break;
        case QCResultManager::QCSysBDS:
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_BDS_B1),FREQ_INDEX_BDS_B1);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_BDS_B2),FREQ_INDEX_BDS_B2);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_BDS_B3),FREQ_INDEX_BDS_B3);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_BDS_B1C),FREQ_INDEX_BDS_B1C);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_BDS_B2A),FREQ_INDEX_BDS_B2A);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_BDS_B2B),FREQ_INDEX_BDS_B2B);
            break;
        case QCResultManager::QCSysGLO:
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GLO_G1),FREQ_INDEX_GLO_G1);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GLO_G2),FREQ_INDEX_GLO_G2);
            break;
        case QCResultManager::QCSysGAL:
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GAL_E1),FREQ_INDEX_GAL_E1);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GAL_E5A),FREQ_INDEX_GAL_E5A);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GAL_E5B),FREQ_INDEX_GAL_E5B);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GAL_E5AB),FREQ_INDEX_GAL_E5AB);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_GAL_E6),FREQ_INDEX_GAL_E6);
            break;
        case QCResultManager::QCSysQZSS:
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_QZSS_L1),FREQ_INDEX_QZSS_L1);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_QZSS_L2),FREQ_INDEX_QZSS_L2);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_QZSS_L5),FREQ_INDEX_QZSS_L5);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_QZSS_L1CP),FREQ_INDEX_QZSS_L1CP);
            ui->freqComboBox->addItem(QCResultManager::getFreqName(sys,FREQ_INDEX_QZSS_L6),FREQ_INDEX_QZSS_L6);
            break;
        case QCResultManager::QCSysComplex:
            ui->freqComboBox->addItem(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP1),(int)QCResultManager::UiComplexMP1);
            ui->freqComboBox->addItem(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP2),(int)QCResultManager::UiComplexMP2);
            ui->freqComboBox->addItem(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP5),(int)QCResultManager::UiComplexMP5);
            ui->freqComboBox->addItem(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP6),(int)QCResultManager::UiComplexMP6);
            ui->freqComboBox->addItem(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP7),(int)QCResultManager::UiComplexMP7);
            ui->freqComboBox->addItem(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP8),(int)QCResultManager::UiComplexMP8);
            ui->freqComboBox->addItem(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexPlotExtend),(int)QCResultManager::UiComplexPlotExtend);
            break;
        default:
            break;
        }
    }
    this->onFreqChanged();
    connect(ui->freqComboBox,&QComboBox::currentTextChanged,this,&SNRAndMPTimePlotDialog::onFreqChanged);

}

void SNRAndMPTimePlotDialog::onFreqChanged()
{
    if(ui->satComboBox->currentIndex() >=0 && ui->freqComboBox->currentIndex() >= 0)
    {
        m_plot->clearFilters();
        SatComboBoxItemData satData = ui->satComboBox->currentData().value<SatComboBoxItemData>();
        QCResultManager::QCSysTypeEnum sys = satData.sys;
        unsigned int prn = satData.prn;
        unsigned int freqIndex = ui->freqComboBox->currentData().toUInt();
        if(QCResultManager::QCSysComplex == sys)
        {
            QVector<QCResultManager::FreqTypeEnum> freqs = QCResultManager::getUIComplexFreqTypes((QCResultManager::UiComplexTypeEnum)freqIndex);
            for(QCResultManager::FreqTypeEnum freq : freqs)
            {
                QCResultManager::GNSSSysFreqIndexInfoStruct freqInfo = QCResultManager::getFreqIndexInfo(freq);
                m_plot->addFilter(freqInfo.sysType,0,freqInfo.freqIndex);
            }
        }
        else
        {
            m_plot->addFilter(sys,prn,freqIndex);
        }

        QTimer::singleShot(0,m_plot,&SNRAndMPTimePlot::updatePlot);
    }
}
