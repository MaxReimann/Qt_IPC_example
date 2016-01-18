
%Module datastreamreader

%Import QtGui/QtGuimod.sip
%Import QtCore/QtCoremod.sip

class DataStreamReader
{

%TypeHeaderCode
#include "datastreamer.h"
%End

public:
	DataStreamReader();

	QVector<QImage> readImages(QSharedMemory &mem);
	void sendImages(QSharedMemory &mem, QVector<QImage> images);
};

