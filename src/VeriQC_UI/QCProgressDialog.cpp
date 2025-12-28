#include "QCProgressDialog.h"
#include "ui_QCProgressDialog.h"
#include "Components/MyMessageBox.h"

QCProgressDialog::QCProgressDialog(QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::QCProgressDialog),m_thread(new QCProcessThread(this))
{
    ui->setupUi(this);
    connect(ui->cancelButton,&QPushButton::clicked,this,&QCProgressDialog::onCancel);
    this->setWindowFlag(Qt::WindowCloseButtonHint,false);
    this->setWindowMinimizeMaximizeButtonVisible(false);

    connect(m_thread,&QCProcessThread::qcProgressRateChanged,this,&QCProgressDialog::updateRate);
    connect(m_thread,&QCProcessThread::qcProcessError,this,&QCProgressDialog::onError);
    connect(m_thread,&QCProcessThread::finished,this,&QCProgressDialog::accept);
}

QCProgressDialog::~QCProgressDialog()
{
    delete ui;
}

void QCProgressDialog::execProcess(const QVector<QSharedPointer<QCObject> > &qcObjects, QCProcessThread::ProcessModeEnum mode)
{
    switch (mode)
    {
    case QCProcessThread::ProcessModeEnum::QCMode:
        this->setWindowTitle(tr("Under QC"));
        break;
    case QCProcessThread::ProcessModeEnum::RinexConvertMode:
        this->setWindowTitle(tr("Rinex Converting"));
        ui->cancelButton->setVisible(false);
        break;
    default:
        break;
    }
    m_thread->setQCObjects(qcObjects);
    m_thread->setProcessMode(mode);
    m_thread->start();
    this->exec();
    m_thread->clearQCObjects();
}

void QCProgressDialog::updateRate(int totalRate, QString currentQCName, int currentQCRate)
{
    ui->totalProgressBar->setValue(totalRate);
    ui->qcObjectNameLabel->setText(currentQCName);
    ui->qcObjectProgressBar->setValue(currentQCRate);
}

void QCProgressDialog::onError(QString currentQCName, QString errorMsg)
{
    MyMessageBox::critical(this,tr("Error"),QString("[%1]\n%2").arg(currentQCName).arg(errorMsg));
}

void QCProgressDialog::onCancel()
{
    auto ret = MyMessageBox::question(this,tr("Cancel"),tr("Do you want to cancel?"));
    if(QMessageBox::Yes == ret)
    {
        m_thread->cancel();
    }
}
