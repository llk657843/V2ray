#include "TcpLatency.h"

#include <QElapsedTimer>
#include <QString>
#include <QTcpSocket>

int TcpLatency::measureConnectMs(const QString& host, int port, int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();
    QTcpSocket socket;
    socket.connectToHost(host, port);
    if (socket.waitForConnected(timeoutMs)) {
        return static_cast<int>(timer.elapsed());
    }
    return -1;
}
