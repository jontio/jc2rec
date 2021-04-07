#include "source.h"
#include <qendian.h>
#include <QDebug>
#include <QTimer>

Source::Source(QString filename,int codec2_mode, int natural, QObject *parent)
:   QIODevice(parent),
  codec2(NULL),
  buf(NULL),
  bits(NULL),
  file(new QFile(this))
{
    qDebug()<<"Source::Source";
    failed=false;
    mode = 0;
    bytesread_sum=0;
    done=false;
    laststatusmsg="";
    mode=codec2_mode;

    codec2 = codec2_create(mode);
    if(codec2==NULL)
    {
        done=true;
        qDebug()<<"cant create codec2. maybe an unsupported mode?";
        laststatusmsg="cant create codec2. maybe an unsupported mode?";
        failed=true;
        return;
    }
    nsam = codec2_samples_per_frame(codec2);
    nbit = codec2_bits_per_frame(codec2);
    buf = (short*)malloc(nsam*sizeof(short));
    nbyte = (nbit + 7) / 8;
    bits = (unsigned char*)malloc(nbyte*sizeof(char));
    codec2_set_natural_or_gray(codec2, !natural);

    qDebug()<<filename;
    file->setFileName(filename);
    if (!file->open(QIODevice::ReadOnly))
    {
        done=true;
        qDebug()<<"cant open file";
        laststatusmsg="cant open file";
        failed=true;
        return;
    }

    if(!failed)laststatusmsg=((QString)"File size is %1 bytes").arg(file->size());

    //fin = fopen(filename.toLocal8Bit().data(),"rb");
   // fout = fopen("c:/testme.raw","wb");



}


Source::~Source()
{
    if(codec2)codec2_destroy(codec2);
    if(buf)free(buf);
    if(bits)free(bits);
    qDebug()<<"Source::~Source()";
}

void Source::start()
{
    open(QIODevice::ReadOnly);
}

void Source::stop()
{
    qDebug()<<"Source::stop";
    close();
    file->close();
}

qint64 Source::readData(char *data, qint64 len)
{
    if(done)
    {
        qDebug()<<"Source is done";
        emit percentage_played(100);
        emit readChannelFinished();
        QTimer::singleShot(0,this,SLOT(stop()));//seems to need to be placed in the message queue on windows else calling close will crash
        return 0;
    }

    int bufptr=0;

    while((bufptr+sizeof(short)*nsam)<(len/2))//(len/2) rather than (len) allows smaller sound card buffer
    {
        //qint64 bytesread=fread(bits, sizeof(char), nbyte, fin);
        qint64 bytesread=file->read((char*)bits, nbyte);
        bytesread_sum+=bytesread;
        emit percentage_played(100*bytesread_sum/file->size());
        if(bytesread==nbyte)
        {
            codec2_decode(codec2, buf, bits);
            //fwrite(buf, sizeof(short), nsam, fout);
            memcpy ( data+bufptr, buf, sizeof(short)*nsam);
        }
        else
        {
            bufptr=0;
            done=true;
            break;
        }
        bufptr+=sizeof(short)*nsam;
    }

    return bufptr;
}

qint64 Source::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

