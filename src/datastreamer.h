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

        stream_recv >> outVec;
        
        sharedMemory_recv.unlock();

        return outVec;
    }

    void sendImages(QSharedMemory &sharedMemory_send, QVector<QImage> images)
    {


        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << images;

        sharedMemory_send.lock();

        memset(sharedMemory_send.data(), 0, sharedMemory_send.size()); //zero memory
        char *to = (char*)sharedMemory_send.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(sharedMemory_send.size(), (int) buffer.size()));

        sharedMemory_send.unlock();


    }
};