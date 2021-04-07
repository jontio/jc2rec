#ifndef SINK_H
#define SINK_H

#include <QIODevice>
#include <QFile>
#include "codec2.h"

class Sink : public QIODevice
{
    Q_OBJECT
public:
    explicit Sink(QString filename, int codec2_mode, int natural, bool save_uncompressed_pcm_too, QObject *parent);
    ~Sink();
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    QString laststatusmsg;
    bool failed;
private:
    int            mode;
    CODEC2        *codec2;
    short         *buf;
    unsigned char *bits, *prev_bits;
    int            nsam, nbit, nbyte;
    QFile          *file;
    QFile          *file_pcm;
    bool           done;
    bool save_uncompressed_pcm_too;
    int bufremptr;
    void writeWavHeader();
signals:
    void signal_volume(int volume);
    void ChannelFailed();
public slots:
    void start();
    void stop();
};

#endif // SINK_H
