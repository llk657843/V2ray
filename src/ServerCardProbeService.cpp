#include "ServerCardProbeService.h"
#include "TcpLatency.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

ServerCardProbeService::ServerCardProbeService(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

void ServerCardProbeService::startProbe(int serverIndex, const QString& host, int port)
{
    if (host.isEmpty() || port <= 0) {
        return;
    }

    auto* watcher = new QFutureWatcher<int>(this);
    connect(watcher, &QFutureWatcher<int>::finished, this, [this, serverIndex, host, watcher]() {
        const int latency = watcher->result();
        watcher->deleteLater();
        emit latencyMeasured(serverIndex, latency);

        const QByteArray enc = QUrl::toPercentEncoding(host);
        const QUrl url(QStringLiteral("http://ip-api.com/json/%1?fields=status,message,country,countryCode,query")
                           .arg(QString::fromUtf8(enc)));
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0 (compatible; V2rayCpp/1.0)"));
        QNetworkReply* reply = m_nam->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, serverIndex, reply]() {
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError) {
                emit geoFinished(serverIndex, QStringLiteral("未知"), QString(), QString(), QString());
                return;
            }

            const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (!doc.isObject()) {
                emit geoFinished(serverIndex, QStringLiteral("未知"), QString(), QString(), QString());
                return;
            }

            const QJsonObject o = doc.object();
            if (o.value(QStringLiteral("status")).toString() != QStringLiteral("success")) {
                const QString msg = o.value(QStringLiteral("message")).toString();
                if (!msg.isEmpty()) {
                    emit geoFinished(serverIndex, msg, QString(), QString(), QString());
                } else {
                    emit geoFinished(serverIndex, QStringLiteral("未知"), QString(), QString(), QString());
                }
                return;
            }

            const QString cty = o.value(QStringLiteral("country")).toString();
            const QString code = o.value(QStringLiteral("countryCode")).toString();
            const QString q = o.value(QStringLiteral("query")).toString();

            QString countryLine;
            if (!cty.isEmpty()) {
                countryLine = code.isEmpty() ? cty : QStringLiteral("%1 (%2)").arg(cty, code);
            }
            if (countryLine.isEmpty()) {
                countryLine = QStringLiteral("未知");
            }

            emit geoFinished(serverIndex, countryLine, q, cty, code);
        });
    });

    QFuture<int> fut = QtConcurrent::run([host, port]() { return TcpLatency::measureConnectMs(host, port); });
    watcher->setFuture(fut);
}
