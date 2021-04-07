#ifndef SOURCE_H
#define SOURCE_H

#include <QIODevice>
#include <QFile>
#include "codec2.h"

class Source : public QIODevice
{
    Q_OBJECT
public:
    explicit Source(QString filename, int codec2_mode, int natural, QObject *parent);
    ~Source();
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    QString laststatusmsg;
    bool failed;
private:
    int            mode;
    CODEC2        *codec2;
    short         *buf;
    unsigned char *bits;
    int            nsam, nbit, nbyte;
    QFile          *file;
    bool           done;
    qint64         bytesread_sum;

   // FILE          *fin;
  //  FILE          *fout;
signals:
    void percentage_played(int percentage);
public slots:
    void start();
    void stop();
};

#endif // SOURCE_H
