#include "receivethread.hpp"
#include "sendthread.hpp"

#include <QtCore/QCoreApplication>
#include <QtCore/qglobal.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ReceiveThread receiveThread;
    receiveThread.onStart();
    SendThread sendThread;
    sendThread.onStart();

    return a.exec();
}
