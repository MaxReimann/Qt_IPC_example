#include "process_example.h"
#include <cmath>
#include <stdexcept>


#define PROCESSNAME QString("process_example3")

#define MAXBUFFERSIZEBYTES 1073741824 //1 gb

void copyToMemory(QSharedMemory& memory, QBuffer& buf, int size)
{
	memory.lock();
    char *to = (char*)memory.data();
    const char *from = buf.data().data();
    memcpy(to, from, qMin(memory.size(), size));
    memory.unlock();
}


QPoint ProcessExample::readCommStatus()
{
	QPoint status;
	QBuffer buf;
    QDataStream stream_recv(&buf);

    sharedMemory_comm.lock();
    buf.setData((char*)sharedMemory_comm.constData(), sharedMemory_comm.size());
    buf.open(QBuffer::ReadOnly);
    stream_recv >> status;
    sharedMemory_comm.unlock();

    return status;
}

void ProcessExample::updateCommStatus(int status)
{

	QPoint entireStatus = readCommStatus();
	entireStatus.setX(status);


	QBuffer buf;
    buf.open(QBuffer::ReadWrite);
	QDataStream out(&buf);
	out << entireStatus;

	copyToMemory(sharedMemory_comm, buf, buf.size());
}

void ProcessExample::updateCommStatusClient(int clientStatus)
{

	QPoint entireStatus = readCommStatus();
	entireStatus.setY(clientStatus);


	QBuffer buf;
    buf.open(QBuffer::ReadWrite);
	QDataStream out(&buf);
	out << entireStatus;

	copyToMemory(sharedMemory_comm, buf, buf.size());
}


//if comparator is true, will wait
int ProcessExample::waitAndPrint(comparefunc compare)
{

    while (compare(readCommStatus()) && pyProcess->waitForReadyRead(-1)){
		QByteArray newData = pyProcess->readAllStandardOutput();
		QString result = QString::fromLocal8Bit(newData);
		qDebug(qPrintable(QString("py:")+result));

		QByteArray errordata = pyProcess->readAllStandardError();
		QString resultErr = QString::fromLocal8Bit(errordata);
		if (QString::compare(resultErr,QString("")) != 0)
		{
			qDebug(qPrintable(QString("py error:")+resultErr));
			qDebug() << "exting because of python error..";
			throw (QString("py error:")+resultErr).toStdString();
		}
	}

	return readCommStatus().y();
}


ProcessExample::ProcessExample()
{

  	QObject *parent;

    //## create communication buffers ##
    QBuffer buffer_comm;
    buffer_comm.open(QBuffer::ReadWrite);
    QDataStream out_comm(&buffer_comm);
    //pair stands for 1: host memory (_in memory) pending buffer size
    //2: pyprocess memory pending buffer size (_out memory)
    out_comm << QPoint(0,0); 

    sharedMemory_comm.setKey(PROCESSNAME + QString("_communication"));
    if (!sharedMemory_comm.create(buffer_comm.size())) {
        qDebug() << "Unable to create shared memory segment for communication memory.";
        qDebug() << sharedMemory_comm.errorString();
        throw std::exception();
    }
    copyToMemory(sharedMemory_comm, buffer_comm, buffer_comm.size());


    //## start process ##
    QString program = "python";
    QStringList arguments;
    QString programPath(PYSOURCE);
    arguments << "-u" << programPath+QString("/test.py");
    qDebug() << arguments;

    pyProcess = new QProcess();
    pyProcess->start(program, arguments);
}


void ProcessExample::sendImages(QVector<QImage>& imgs)
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);

    updateCommStatus(0);

    out << imgs;

    int size = buffer.size();

    if (!sharedMemory_send.isAttached())
    {   
        sharedMemory_send.setKey(PROCESSNAME + QString("_in"));
        if (!sharedMemory_send.create(size)) {
            qDebug() << "Unable to create shared memory segment.";
            qDebug() << sharedMemory_send.errorString();
            throw std::exception();
        }
    }

    qDebug() << "sizeof mem: " << sharedMemory_send.size() << "bufsize:" << buffer.size();

    memset(sharedMemory_send.data(), 0, sharedMemory_send.size()); //zero memory
    copyToMemory(sharedMemory_send, buffer, size);

    updateCommStatus(size); // client ready to receive

    qDebug() << "image copied to shared memory";
    
    waitAndPrint([](QPoint p){return p.y() == 0;}); //waits until client has pending data
}


QVector<QImage> ProcessExample::receiveImages()
{
	qDebug() << "ok now to second part..";

    int pending = readCommStatus().y();
    if (!sharedMemory_recv.isAttached())
    {
        sharedMemory_recv.setKey(PROCESSNAME + QString("_out"));

        if (!sharedMemory_recv.create(pending)) {
            qDebug() << "Unable to create shared memory segment.";
    		qDebug() << sharedMemory_recv.errorString();
            throw std::exception();
        }
    }
    qDebug() << "pending" << pending << "recv mem size:" << sharedMemory_recv.size();


    QBuffer buffer_recv;
    QDataStream stream_recv(&buffer_recv);

    //wait for pending to be 0
    waitAndPrint([](QPoint p){return p.y() > 0;});

    sharedMemory_recv.lock();
    buffer_recv.setData((char*)sharedMemory_recv.constData(), sharedMemory_recv.size());
    buffer_recv.open(QBuffer::ReadOnly);

    QVector<QImage> outVec;
    stream_recv >> outVec;
    
    sharedMemory_recv.unlock();

    qDebug() << "received " << outVec.size() << "images";

    return outVec;
}



ProcessExample::~ProcessExample()
{
    sharedMemory_recv.detach();
    sharedMemory_comm.detach();
    sharedMemory_send.detach();
    qDebug() << "calling destructor";
}


