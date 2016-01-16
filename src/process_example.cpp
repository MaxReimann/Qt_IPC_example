#include <QProcess>
#include <QtDebug>
#include <QBuffer>
#include <QImage>
#include <QSharedMemory>
#include <QPoint>

#define PROCESSNAME QString("process_example")

void copyToMemory(QSharedMemory& memory, QBuffer& buf, int size)
{
	memory.lock();
    char *to = (char*)memory.data();
    const char *from = buf.data().data();
    memcpy(to, from, qMin(memory.size(), size));
    memory.unlock();
}


QPoint readCommStatus(QSharedMemory& memory)
{
	QPoint status;
	QBuffer buf;
    QDataStream stream_recv(&buf);

    memory.lock();
    buf.setData((char*)memory.constData(), memory.size());
    buf.open(QBuffer::ReadOnly);
    stream_recv >> status;
    memory.unlock();

    return status;
}

void updateCommStatus(QSharedMemory& memory, int status)
{

	QPoint entireStatus = readCommStatus(memory);
	entireStatus.setX(status);


	QBuffer buf;
    buf.open(QBuffer::ReadWrite);
	QDataStream out(&buf);
	out << entireStatus;

	copyToMemory(memory, buf, buf.size());
}

void updateCommStatusClient(QSharedMemory& memory, int clientStatus)
{

	QPoint entireStatus = readCommStatus(memory);
	entireStatus.setY(clientStatus);


	QBuffer buf;
    buf.open(QBuffer::ReadWrite);
	QDataStream out(&buf);
	out << entireStatus;

	copyToMemory(memory, buf, buf.size());
}


int waitAndPrint(QProcess* pyProcess, QSharedMemory& sharedMemory_comm)
{
	int pending = 0;
    while (pyProcess->waitForReadyRead(-1) && pending == 0){
		QByteArray newData = pyProcess->readAllStandardOutput();
		QString result = QString::fromLocal8Bit(newData);
		qDebug(qPrintable(QString("py:")+result));

		QByteArray errordata = pyProcess->readAllStandardError();
		QString resultErr = QString::fromLocal8Bit(errordata);
		if (QString::compare(resultErr,QString("")) != 0)
		{
			qDebug(qPrintable(QString("py error:")+resultErr));
			qDebug() << "exting because of python error..";
			return -1;
		}

		pending = readCommStatus(sharedMemory_comm).y();
	}

	return pending;
}

int main(int argc, const char* argv[])
{
  	QObject *parent;
    QImage image("../test.png");
    if (image.isNull())
    {
    	qDebug() << " image could not be loaded";
    	return 0;
    }


    QBuffer buffer_comm;
    buffer_comm.open(QBuffer::ReadWrite);
    QDataStream out_comm(&buffer_comm);
    //pair stands for 1: host memory (_in memory) pending buffer size (unused) 
    //2: pyprocess memory pending buffer size (_out memory)
    out_comm << QPoint(0,0); 


    QSharedMemory sharedMemory_comm(PROCESSNAME + QString("_communication"));
    if (!sharedMemory_comm.create(buffer_comm.size())) {
        qDebug() << "Unable to create shared memory segment for communication memory.";
        return 0;
    }

  
    copyToMemory(sharedMemory_comm, buffer_comm, buffer_comm.size());




    QString program = "python";
    QStringList arguments;
    QString programPath(PYSOURCE);
    arguments << "-u" << programPath+QString("/test.py");
    qDebug() << arguments;

    QProcess *pyProcess = new QProcess();
    pyProcess->start(program, arguments);

    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);
    out << image;
    // out << image2;

    int size = buffer.size();

    QSharedMemory sharedMemory(PROCESSNAME + QString("_in"));
    if (!sharedMemory.create(size)) {
        qDebug() << "Unable to create shared memory segment.";
        return 0;
    }

    qDebug() << "sizeof mem: " << sharedMemory.size() << "bufsize:" << buffer.size();

    copyToMemory(sharedMemory, buffer, size);

    qDebug() << "image copied to shared memory";
    int result = waitAndPrint(pyProcess,sharedMemory_comm);
	sharedMemory.detach();

    if (result < 0)
    	return -1;

  	int pending = result;

	qDebug() << "ok now to second part..";

	QSharedMemory sharedMemory_recv(PROCESSNAME + QString("_out"));


    if (!sharedMemory_recv.create(pending)) {
        qDebug() << "Unable to create shared memory segment.";
		qDebug() << sharedMemory_recv.errorString();
        return 0;
    }
    qDebug() << "pending" << sharedMemory_recv.size();
	// set pending data back to 0
	updateCommStatusClient(sharedMemory_comm, 0);


	QBuffer buffer_recv;
    QDataStream stream_recv(&buffer_recv);
    QImage image_in;

    result = waitAndPrint(pyProcess,sharedMemory_comm);

    sharedMemory_recv.lock();
    buffer_recv.setData((char*)sharedMemory_recv.constData(), sharedMemory_recv.size());
    buffer_recv.open(QBuffer::ReadOnly);
    stream_recv >> image_in;
    sharedMemory_recv.unlock();


    
    qDebug() << "received image width" << image_in.width();

    sharedMemory_recv.detach();
    sharedMemory_comm.detach();


	return 0;
}