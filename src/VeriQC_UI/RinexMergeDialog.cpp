#include "RinexMergeDialog.h"
#include "ui_RinexMergeDialog.h"
#include <QFileDialog>
#include "Components/MyMessageBox.h"
#include <QProgressDialog>
#include "VeriQC.h"
#include "RinexMergeProcessor.h"

RinexMergeDialog::RinexMergeDialog(QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::RinexMergeDialog)
{
    ui->setupUi(this);

    connect(ui->inputFileAddButton,&QPushButton::clicked,this,&RinexMergeDialog::onAddInputFiles);
    connect(ui->outputDirOpenButton,&QPushButton::clicked,this,&RinexMergeDialog::onOpenOutputDir);
    connect(ui->mergeButton,&QPushButton::clicked,this,&RinexMergeDialog::onMerge);
    connect(ui->cancelButton,&QPushButton::clicked,this,&RinexMergeDialog::reject);
    connect(ui->inputFilePathsTextEdit,&QPlainTextEdit::textChanged,this,&RinexMergeDialog::onInputFilePathsChanged);
}

RinexMergeDialog::~RinexMergeDialog()
{
    delete ui;
}

void RinexMergeDialog::onAddInputFiles()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this,QString(),QString(),"Rinex (*.*o *。*O)");
    if(!filePaths.empty())
    {
        QString currentText = ui->inputFilePathsTextEdit->toPlainText();
        if(!currentText.isEmpty())
        {
            currentText += "\n";
        }
        currentText += filePaths.join("\n");
        ui->inputFilePathsTextEdit->setPlainText(currentText);
    }
}

void RinexMergeDialog::onOpenOutputDir()
{
    QString outputDir = QFileDialog::getExistingDirectory(this);
    if(!outputDir.isEmpty())
    {
        ui->outputDirLineEdit->setText(outputDir);
    }
}

void RinexMergeDialog::onMerge()
{
    QByteArray inputFilePathsText = ui->inputFilePathsTextEdit->toPlainText().toLocal8Bit();
    if(inputFilePathsText.size() > (MAX_FILE_NAME_LENGTH - 1))
    {
        MyMessageBox::critical(this,tr("Error"),tr("Total length of input File Paths is out of maximum limit"));
        return;
    }
    QStringList inputFilePaths = ui->inputFilePathsTextEdit->toPlainText().split("\n",QString::SkipEmptyParts);
    if(inputFilePaths.isEmpty())
    {
        MyMessageBox::critical(this,tr("Error"),tr("Please specify the files to merge"));
        return;
    }
    if(inputFilePaths.size() > 10)
    {
        MyMessageBox::critical(this,tr("Error"),tr("The maximum supported file merging is 10 files"));
        return;
    }
    QString outputDir = ui->outputDirLineEdit->text();
    if(outputDir.isEmpty())
    {
        MyMessageBox::critical(this,tr("Error"),tr("Please specify the output directory"));
        return;
    }

    RinexMergeProcessor processor;

    QProgressDialog progressDialog(this);
    progressDialog.setWindowTitle(tr("Rinex Merging"));
    progressDialog.setWindowFlag(Qt::WindowCloseButtonHint,false);
    progressDialog.setWindowFlag(Qt::WindowContextHelpButtonHint,false);
    progressDialog.setLabelText(tr("Rinex Merging"));
    progressDialog.setCancelButton(nullptr);

    connect(&processor,&RinexMergeProcessor::rateChanged,&progressDialog,&QProgressDialog::setValue);
    connect(&processor,&RinexMergeProcessor::finished,&progressDialog,&QDialog::accept);
    processor.startProcess(inputFilePaths,outputDir);
    progressDialog.exec();
    MyMessageBox::information(this,tr("Done"),tr("Rinex merge finished"));
}

void RinexMergeDialog::onInputFilePathsChanged()
{
    QString currentText = ui->inputFilePathsTextEdit->toPlainText();
    QStringList filePaths = currentText.split("\n",QString::SkipEmptyParts);
    if(filePaths.isEmpty())
    {
        ui->outputDirLineEdit->clear();
    }
    else
    {
        QFileInfo firstFileInfo(filePaths[0]);
        if(firstFileInfo.exists() && firstFileInfo.isFile())
        {
            ui->outputDirLineEdit->setText(firstFileInfo.absolutePath());
        }
    }
}
