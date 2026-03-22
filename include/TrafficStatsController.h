#pragma once

#include <QObject>

class QTimer;

/// 底部速度展示（当前为演示用随机值）；与真实流量统计解耦，后续可替换数据源
class TrafficStatsController : public QObject
{
    Q_OBJECT

public:
    explicit TrafficStatsController(QObject* parent = nullptr);

    void start();
    void stop();

signals:
    void speedsUpdated(const QString& downloadText, const QString& uploadText);

private:
    void onTick();

    QTimer* m_timer = nullptr;
    int m_tick = 0;
};
