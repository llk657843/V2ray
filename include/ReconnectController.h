#pragma once

#include <QObject>

class QTimer;

/// 核心异常退出后的延迟重连（仅发信号，由界面决定如何启动）
class ReconnectController : public QObject
{
    Q_OBJECT

public:
    explicit ReconnectController(QObject* parent = nullptr);

    void arm(int intervalMs = 5000);
    void disarm();

signals:
    void reconnectDue();

private:
    QTimer* m_timer = nullptr;
};
