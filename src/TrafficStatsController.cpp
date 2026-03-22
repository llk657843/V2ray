#include "TrafficStatsController.h"
#include <QTimer>

TrafficStatsController::TrafficStatsController(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TrafficStatsController::onTick);
}

void TrafficStatsController::start()
{
    m_tick = 0;
    m_timer->start(1000);
}

void TrafficStatsController::stop()
{
    m_timer->stop();
    emit speedsUpdated(QStringLiteral("0 KB/s"), QStringLiteral("0 KB/s"));
}

void TrafficStatsController::onTick()
{
    ++m_tick;

    const qint64 downloadSpeed = (100 + (m_tick * 37) % 5000) * 1024;
    const qint64 uploadSpeed = (50 + (m_tick * 23) % 1000) * 1024;

    QString downloadStr;
    if (downloadSpeed >= 1024 * 1024) {
        downloadStr = QStringLiteral("%1 MB/s").arg(downloadSpeed / (1024.0 * 1024.0), 0, 'f', 1);
    } else {
        downloadStr = QStringLiteral("%1 KB/s").arg(downloadSpeed / 1024.0, 0, 'f', 1);
    }

    QString uploadStr;
    if (uploadSpeed >= 1024 * 1024) {
        uploadStr = QStringLiteral("%1 MB/s").arg(uploadSpeed / (1024.0 * 1024.0), 0, 'f', 1);
    } else {
        uploadStr = QStringLiteral("%1 KB/s").arg(uploadSpeed / 1024.0, 0, 'f', 1);
    }

    emit speedsUpdated(downloadStr, uploadStr);
}
