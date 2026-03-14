#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QSslCertificate>

class XrayApiClient : public QObject
{
    Q_OBJECT

public:
    explicit XrayApiClient(QObject* parent = nullptr);
    ~XrayApiClient();

    void setServer(const QString& host, quint16 port);
    void setSslEnabled(bool enabled, const QString& caCertPath = QString());
    void setApiKey(const QString& apiKey);

    void testConnection();
    void addInbound(const QJsonObject& inbound);
    void removeInbound(const QString& tag);
    void listInbounds();
    void addOutbound(const QJsonObject& outbound);
    void removeOutbound(const QString& tag);
    void listOutbounds();

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void listInboundsResponse(const QJsonObject& response);
    void listOutboundsResponse(const QJsonObject& response);
    void addInboundResponse(const QJsonObject& response);
    void addOutboundResponse(const QJsonObject& response);

private slots:
    void onReplyFinished();

private:
    void sendRequest(const QString& method, const QString& path, const QJsonObject& params = QJsonObject());
    QString buildUrl(const QString& path) const;
    void handleResponse(QNetworkReply* reply);

    QNetworkAccessManager* m_networkManager;
    QString m_host;
    quint16 m_port;
    bool m_sslEnabled;
    QString m_caCertPath;
    QString m_apiKey;
};
