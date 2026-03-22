#include "ReconnectController.h"
#include <QTimer>

ReconnectController::ReconnectController(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this]() { emit reconnectDue(); });
}

void ReconnectController::arm(int intervalMs)
{
    m_timer->start(intervalMs);
}

void ReconnectController::disarm()
{
    m_timer->stop();
}
