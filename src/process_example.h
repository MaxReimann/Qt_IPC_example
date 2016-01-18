#pragma once

#include <QProcess>
#include <QtDebug>
#include <QBuffer>
#include <QImage>
#include <QSharedMemory>
#include <QPoint>
#include <QObject>  

typedef bool (*comparefunc)(QPoint);
class ProcessExample  : public QObject
{
	Q_OBJECT

	public: 
		ProcessExample();
		~ProcessExample();
		void process();
		void sendImages(QVector<QImage>& imgs);
		QVector<QImage> receiveImages();

	private:
		QPoint readCommStatus();
		void updateCommStatus(int status);
		void updateCommStatusClient(int status);
		int waitAndPrint(comparefunc comp);

		QProcess *pyProcess;

		QSharedMemory sharedMemory_recv;
    	QSharedMemory sharedMemory_comm;
    	QSharedMemory sharedMemory_send;

};