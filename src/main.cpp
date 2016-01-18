#include "process_example.h"


int main(int argc, const char* argv[])
{
    QImage image("../test.png");
    QImage image2("../cat.png");
    if (image.isNull() || image2.isNull())
    {
        qDebug() << " image could not be loaded";
        return 0;
    }

    QVector<QImage> images;
    for (int i = 0; i  < 100; i++)
        images.push_back(image.copy());
    images.push_back(image2);


    QVector<QImage> received;
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

    received.back().save("received.png","PNG");

    qDebug() << "received"    << received.size() << "images";

    return 0;
}