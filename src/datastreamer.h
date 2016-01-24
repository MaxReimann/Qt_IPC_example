#include <QImage>
#include <QSharedMemory>
#include <QVector>
#include <QBuffer>

class DataStreamReader
{

public:
    DataStreamReader(){};
    QVector<QImage> readImages(QSharedMemory &sharedMemory_recv)
    {
        // QImage outVec;
        QBuffer buffer_recv;
        QDataStream stream_recv(&buffer_recv);
        sharedMemory_recv.lock();
        buffer_recv.setData((char*)sharedMemory_recv.constData(), sharedMemory_recv.size());
        buffer_recv.open(QBuffer::ReadOnly);


        QVector<QImage> outVec;

        int numImages;
        stream_recv >> numImages;

        for (int i= 0; i < numImages; i++)
        {
            int format;
            int width, height, bytesPerLine;
            unsigned int byteCount;

            stream_recv >> format;
            stream_recv >> width;
            stream_recv >> height;
            stream_recv >> bytesPerLine;
            QImage img(width, height, (QImage::Format) format);
            img.fill(0);
            char *p = (char*) img.bits();
            stream_recv.readBytes(p,  byteCount);
            // stream_recv.readBytes(img.bits(), byteCount); //no encoding

            outVec.push_back(img);
        }

        // stream_recv >> outVec;
        
        sharedMemory_recv.unlock();

        return outVec;
    }

    void sendImages(QSharedMemory &sharedMemory_send, QVector<QImage> images)
    {


        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);

        out << (int) images.size();

        for (int i = 0; i < images.size();i++)
        {
            QImage &img = images[i];
            out << (int) img.format();
            out << img.width();
            out << img.height();
            out << img.bytesPerLine();
            out.writeBytes((char *) img.constBits(), (unsigned int) img.byteCount()); //no encoding
        }

        // out << images;

        sharedMemory_send.lock();

        memset(sharedMemory_send.data(), 0, sharedMemory_send.size()); //zero memory
        char *to = (char*)sharedMemory_send.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(sharedMemory_send.size(), (int) buffer.size()));

        sharedMemory_send.unlock();


    }
};