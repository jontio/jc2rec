#include "sink.h"
#include <qendian.h>
#include <QDebug>
#include <math.h>
#include <QDataStream>
#include <QTimer>

Sink::Sink(QString filename, int codec2_mode, int natural, bool save_uncompressed_pcm_too_, QObject *parent)
:   QIODevice(parent),
    codec2(NULL),
    buf(NULL),
    bits(NULL),
    file(new QFile(this)),
    file_pcm(new QFile(this))
{
    qDebug()<<"Sink::Sink()";

    save_uncompressed_pcm_too=save_uncompressed_pcm_too_;
    failed=false;
    laststatusmsg="";
    bufremptr=0;
    mode = 0;
    done=false;
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
    if (!file->open(QIODevice::WriteOnly))
    {
        done=true;
        qDebug()<<"cant open file";
        laststatusmsg="cant open file for saving";
        failed=true;
        return;
    }

    file_pcm->setFileName(filename+".wav");
    if (save_uncompressed_pcm_too)
    {
        if(!file_pcm->open(QIODevice::WriteOnly))
        {
            done=true;
            qDebug()<<"cant open file";
            laststatusmsg="cant open uncompressed pcm file for saving";
            failed=true;
            return;
        }
        else writeWavHeader();//make space
    }


}

Sink::~Sink()
{
    if(codec2)codec2_destroy(codec2);
    if(buf)free(buf);
    if(bits)free(bits);
    qDebug()<<"Sink::~Sink()";
}

void Sink::start()
{
    open(QIODevice::WriteOnly);
}

void Sink::stop()
{
    qDebug()<<"Sink::stop()";
    close();
    file->close();
    if(save_uncompressed_pcm_too)writeWavHeader();//fill file sz
    file_pcm->close();
}


qint64 Sink::readData(char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 Sink::writeData(const char *data, qint64 len)
{

    if(failed)emit ChannelFailed();

    if(done)
    {
        QTimer::singleShot(0,this,SLOT(stop()));
        return 0;
    }

    //emit max volume in %
    const short *ptr = reinterpret_cast<const short *>(data);
    int maxval=0;
    for(int i=0;i<len/sizeof(short);i++)
    {
        int val=abs((int)(*ptr));
        if(val>maxval)maxval=val;
        ptr++;
    }
    emit signal_volume(100*maxval/32767);

    if(save_uncompressed_pcm_too)file_pcm->write(data,len);

    //fill "buf" till we have enough to decode then write to file
    for(int bufptr=0;bufptr<len;)
    {

        int bytestoread=qMin(((qint64)(sizeof(short)*nsam-bufremptr)),len-bufptr);//how much can we read?

        if(bytestoread!=((qint64)(sizeof(short)*nsam-bufremptr)))//not enough? then save for later
        {
            memcpy (buf+bufremptr/sizeof(short), data+bufptr,bytestoread);
            bufremptr+=bytestoread;
            bufptr+=bytestoread;
            break;
        }
        else//enough? then decode
        {
            memcpy (buf+bufremptr/sizeof(short), data+bufptr, bytestoread);
            codec2_encode(codec2,bits,buf);
            file->write((char*)bits,nbyte);
            bufremptr=0;
        }

        bufptr+=bytestoread;
    }

    return len;

}


//just so the wav file has a header
void Sink::writeWavHeader()
{
file_pcm->seek(0);
QDataStream out(file_pcm);
out.setByteOrder(QDataStream::LittleEndian);

// RIFF chunk
out.writeRawData("RIFF", 4);
out << quint32(file_pcm->size()-8); // RIFF chunk size
out.writeRawData("WAVE", 4);

// Format description chunk
out.writeRawData("fmt ", 4);
out << quint32(16); // "fmt " chunk size (always 16 for PCM)
out << quint16(1); // data format (1 => PCM)
out << quint16(1);
out << quint32(8000);
out << quint32(8000 * 1 * 16 / 8 ); // bytes per second
out << quint16(1 * 16 / 8); // Block align
out << quint16(16); // Significant Bits Per Sample

// Data chunk
out.writeRawData("data", 4);
out << quint32(file_pcm->size()-44); // data chunk size

Q_ASSERT(pos() == 44); // Must be 44 for WAV PCM
}



