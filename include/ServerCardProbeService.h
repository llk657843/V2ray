#pragma once

#include <QObject>

class QNetworkAccessManager;

/// 后台测 TCP 延迟并查询 ip-api 地理信息；不操作 UI，由槽更新卡片与 ProfileItem
class ServerCardProbeService : public QObject
{
    Q_OBJECT

public:
    explicit ServerCardProbeService(QObject* parent = nullptr);

    void startProbe(int serverIndex, const QString& host, int port);

signals:
    void latencyMeasured(int serverIndex, int latencyMs);
    /// primary/secondary 用于卡片；country、countryCode 用于写回 Profile（失败时 country 为空）
    void geoFinished(int serverIndex, const QString& primaryLine, const QString& secondaryLine,
                     const QString& country, const QString& countryCode);

private:
    QNetworkAccessManager* m_nam = nullptr;
};
