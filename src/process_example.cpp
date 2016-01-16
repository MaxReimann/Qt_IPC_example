#include <QProcess>
#include <QtDebug>
#include <QBuffer>
#include <QImage>
#include <QSharedMemory>

int main(int argc, const char* argv[])
{
    QObject *parent;
    QImage image("../test.png");
    if (image.isNull())
    {
    	qDebug() << " image could not be loaded";
    	return 0;
    }

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

    int size = buffer.size();

    QSharedMemory sharedMemory("process_example");
    if (!sharedMemory.create(size)) {
        qDebug() << "Unable to create shared memory segment.";
        return 0;
    }

    qDebug() << "sizeof mem: " << sharedMemory.size() << "bufsize:" << buffer.size();
    sharedMemory.lock();
    qDebug() << "shared memory locked";


    char *to = (char*)sharedMemory.data();
    const char *from = buffer.data().data();
    memcpy(to, from, qMin(sharedMemory.size(), size));
    
    sharedMemory.unlock();

    qDebug() << "image copied to shared memory";

    while (pyProcess->waitForReadyRead(-1)){
	QByteArray newData = pyProcess->readAllStandardOutput();
	QString result = QString::fromLocal8Bit(newData);
	qDebug(qPrintable(QString("py:")+result));
    	
    }
    sharedMemory.detach();
    
    return 0;
}
