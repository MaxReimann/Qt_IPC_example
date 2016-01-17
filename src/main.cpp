#include "process_example.h"


int main(int argc, const char* argv[])
{
    QImage image("../test.png");
    if (image.isNull())
    {
        qDebug() << " image could not be loaded";
        return 0;
    }

    std::vector<QImage> images;
    images.push_back(image);


    ProcessExample procHandler;
    procHandler.sendImages(images);
    std::vector<QImage> received = procHandler.receiveImages();

    qDebug() << "received"    << received.size() << "images";

	return 0;
}