#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_source(NULL),
    m_sink(NULL),
    m_audioOutput(NULL),
    m_audioInput(NULL)

{
    ui->setupUi(this);

    //select default devices and set the format
    m_device_out=QAudioDeviceInfo::defaultOutputDevice();
    m_device_in=QAudioDeviceInfo::defaultInputDevice();
    m_format.setSampleRate(8000);
    m_format.setChannelCount(1);
    m_format.setSampleSize(16);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::max_mic_volume(int percentage)
{
    ui->progressBar->setValue(percentage);
}

void MainWindow::played_file_percentage(int percentage)
{
    ui->progressBar->setValue(percentage);
}

void MainWindow::on_openfiletoolButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Raw Codec2 File"), "", tr("Raw Codec2 Files (*.c2 *.raw);;All files (*.*)"));
    ui->playfilelineEdit->setText(fileName);
}

void MainWindow::on_savefiletoolButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save Raw Codec2 File"), "", tr("Raw Codec2 File (*.c2);;Raw Codec2 File (*.raw);;All files (*.*)"));
    ui->savefilelineEdit->setText(fileName);
}

//encode on the fly and output to file using the class called "Sink" to do the work
void MainWindow::on_recButton_clicked()
{
    on_stopButton_clicked();
    if(m_audioInput)m_audioInput->deleteLater();
    if(m_sink)m_sink->deleteLater();

    int mode=0;
    if (ui->bitratecomboBox->currentText()=="3200")
        mode = CODEC2_MODE_3200;
    else if (ui->bitratecomboBox->currentText()=="2400")
        mode = CODEC2_MODE_2400;
    else if (ui->bitratecomboBox->currentText()=="1600")
        mode = CODEC2_MODE_1600;
    else if (ui->bitratecomboBox->currentText()=="1400")
        mode = CODEC2_MODE_1400;
    else if (ui->bitratecomboBox->currentText()=="1300")
        mode = CODEC2_MODE_1300;
    else if (ui->bitratecomboBox->currentText()=="1200")
        mode = CODEC2_MODE_1200;
    else if (ui->bitratecomboBox->currentText()=="700C")
        mode = CODEC2_MODE_700C;
    else if (ui->bitratecomboBox->currentText()=="450")
        mode = CODEC2_MODE_450;
    else if (ui->bitratecomboBox->currentText()=="450PWB")
        mode = CODEC2_MODE_450PWB;
    else {
        qDebug()<<"invalid bit rate";
    }

    int natural=1;
    if(ui->encodingcomboBox->currentText()=="Natural") natural=1;
    if(ui->encodingcomboBox->currentText()=="Gray") natural=0;

    //setup
    m_device_in=QAudioDeviceInfo::defaultInputDevice();
    m_sink = new Sink(ui->savefilelineEdit->text(),mode,natural,ui->saveorgpcmcheckBox->isChecked(), this);
    m_audioInput = new QAudioInput(m_device_in, m_format, this);
    m_audioInput->setBufferSize(8000);//1 second buffer
    connect(m_sink, SIGNAL(signal_volume(int)),this, SLOT(max_mic_volume(int)));
    connect(m_sink, SIGNAL(ChannelFailed()),this,SLOT(on_stopButton_clicked()),Qt::QueuedConnection);
    ui->progressBar->setFormat("Volume");
    ui->recButton->setEnabled(false);
    ui->playButton->setEnabled(false);

    //start
    m_sink->start();
    m_audioInput->start(m_sink);

    ui->statusBar->clearMessage();
    if(!m_sink->laststatusmsg.isEmpty())ui->statusBar->showMessage("Rec: "+m_sink->laststatusmsg);

    if(m_sink->failed)on_stopButton_clicked();

}

//decode on the fly and output to soundcard using the class called "Source" to do the work
void MainWindow::on_playButton_clicked()
{
    on_stopButton_clicked();
    if(m_audioOutput)m_audioOutput->deleteLater();
    if(m_source)m_source->deleteLater();

    int mode=0;
    if (ui->bitratecomboBox->currentText()=="3200")
        mode = CODEC2_MODE_3200;
    else if (ui->bitratecomboBox->currentText()=="2400")
        mode = CODEC2_MODE_2400;
    else if (ui->bitratecomboBox->currentText()=="1600")
        mode = CODEC2_MODE_1600;
    else if (ui->bitratecomboBox->currentText()=="1400")
        mode = CODEC2_MODE_1400;
    else if (ui->bitratecomboBox->currentText()=="1300")
        mode = CODEC2_MODE_1300;
    else if (ui->bitratecomboBox->currentText()=="1200")
        mode = CODEC2_MODE_1200;
    else if (ui->bitratecomboBox->currentText()=="700C")
        mode = CODEC2_MODE_700C;
    else if (ui->bitratecomboBox->currentText()=="450")
        mode = CODEC2_MODE_450;
    else if (ui->bitratecomboBox->currentText()=="450PWB")
        mode = CODEC2_MODE_450PWB;
    else {
        qDebug()<<"invalid bit rate";
    }

    int natural=1;
    if(ui->encodingcomboBox->currentText()=="Natural") natural=1;
    if(ui->encodingcomboBox->currentText()=="Gray") natural=0;

    //setup
    m_device_out=QAudioDeviceInfo::defaultOutputDevice();
    m_source = new Source(ui->playfilelineEdit->text(),mode,natural, this);
    m_audioOutput = new QAudioOutput(m_device_out, m_format, this);
    m_audioOutput->setBufferSize(8000);//1 second buffer
    connect(m_source, SIGNAL(percentage_played(int)),this, SLOT(max_mic_volume(int)));
    connect(m_source, SIGNAL(readChannelFinished()),this,SLOT(on_stopButton_clicked()),Qt::QueuedConnection);//seems to need to be placed in the message queue on windows else calling close will crash
    ui->progressBar->setFormat("Progress");
    ui->recButton->setEnabled(false);
    ui->playButton->setEnabled(false);

    //start
    m_source->start();
    m_audioOutput->start(m_source);

    ui->statusBar->clearMessage();
    if(!m_source->laststatusmsg.isEmpty())ui->statusBar->showMessage("Play: "+m_source->laststatusmsg);

    if(m_source->failed)on_stopButton_clicked();

}

void MainWindow::on_stopButton_clicked()
{
    if(m_audioOutput)m_audioOutput->stop();
    if(m_source)m_source->stop();
    if(m_audioInput)m_audioInput->stop();
    if(m_sink)m_sink->stop();
    max_mic_volume(0);
    ui->progressBar->setFormat("  Volume/Progress");
    ui->recButton->setEnabled(true);
    ui->playButton->setEnabled(true);
    //ui->statusBar->clearMessage();
}

void MainWindow::error_slot_msg(QString msg)
{
    ui->statusBar->showMessage(msg);
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_action_About_triggered()
{
    QMessageBox::about(this,"JC2Rec",""
                                     "<H1>A speech recorder using Codec2</H1>"
                                     "<p>This is a simple program to record and play using the Codec2 codec.</p>"
                                     "<p>Codec2 is an audio codec designed for speech that highly compresses speech."
                                     " For information about the codec see  <a href=\"http://www.rowetel.com/blog/?page_id=452\">http://www.rowetel.com/blog/?page_id=452</a></p>"
                                     "<p>For this application see <a href=\"http://jontio.zapto.org/hda1/jc2rec.html\">http://jontio.zapto.org/hda1/jc2rec.html</a>."
                                     " In addition to this GUI being useful, it is hoped that the source code of it will be useful for others to implement digital communication software.</p>"
                                     "<p>The compressed audio files are saved without any header information so will require you to"
                                     " remember the settings that they were recorded at.</p>Jonti 2015" );
}


