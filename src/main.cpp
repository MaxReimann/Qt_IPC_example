#include "process_example.h"


int main(int argc, const char* argv[])
{
    QImage image("../test.png");
    QImage image2("../screen.png");
    if (image.isNull() || image2.isNull())
    {
        qDebug() << " image could not be loaded";
        return 0;
    }

    std::vector<QImage> images;
    //images.push_back(image);
    images.push_back(image2);

    std::vector<QImage> received;
    ProcessExample procHandler;
    try
    {
        procHandler.sendImages(images);
        received = procHandler.receiveImages();
    }
    catch(std::exception e)
    {
        qDebug() << e.what();
        return -1;
    }

    qDebug() << "received"    << received.size() << "images";

	return 0;
}