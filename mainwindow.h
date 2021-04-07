#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioOutput>
#include <QAudioInput>
#include "codec2.h"
#include "source.h"//from file
#include "sink.h"//to file

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_openfiletoolButton_clicked();

    void on_savefiletoolButton_clicked();

    void on_playButton_clicked();

    void on_stopButton_clicked();

    void on_recButton_clicked();

    void max_mic_volume(int percentage);
    void played_file_percentage(int percentage);
    void on_actionAbout_Qt_triggered();

    void on_action_About_triggered();

    void error_slot_msg(QString msg);

private:
    Ui::MainWindow *ui;
    Source *m_source;
    Sink *m_sink;



    QAudioFormat m_format;
    QAudioOutput *m_audioOutput;
    QAudioInput *m_audioInput;
    QAudioDeviceInfo m_device_out,m_device_in;
};

#endif // MAINWINDOW_H
