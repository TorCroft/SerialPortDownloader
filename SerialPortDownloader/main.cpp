#include "SerialPortDownloader.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SerialPortDownloader w;
    w.show();
    return a.exec();
}
