#include "XrayApiClient.h"
#include <QUrl>
#include <QDebug>

XrayApiClient::XrayApiClient(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_port(0)
    , m_sslEnabled(false)
{
}

XrayApiClient::~XrayApiClient()
{
}

void XrayApiClient::setServer(const QString& host, quint16 port)
{
    m_host = host;
    m_port = port;
}

void XrayApiClient::setSslEnabled(bool enabled, const QString& caCertPath)
{
    m_sslEnabled = enabled;
    m_caCertPath = caCertPath;
}

void XrayApiClient::setApiKey(const QString& apiKey)
{
    m_apiKey = apiKey;
}

void XrayApiClient::testConnection()
{
    listInbounds();
}

void XrayApiClient::addInbound(const QJsonObject& inbound)
{
    QJsonObject params;
    params["inbound"] = inbound;
    sendRequest("POST", "/v2ray/command/add_inbound", params);
}

void XrayApiClient::removeInbound(const QString& tag)
{
    QJsonObject params;
    params["tag"] = tag;
    sendRequest("POST", "/v2ray/command/remove_inbound", params);
}

void XrayApiClient::listInbounds()
{
    sendRequest("POST", "/v2ray/command/list_inbounds", QJsonObject());
}

void XrayApiClient::addOutbound(const QJsonObject& outbound)
{
    QJsonObject params;
    params["outbound"] = outbound;
    sendRequest("POST", "/v2ray/command/add_outbound", params);
}

void XrayApiClient::removeOutbound(const QString& tag)
{
    QJsonObject params;
    params["tag"] = tag;
    sendRequest("POST", "/v2ray/command/remove_outbound", params);
}

void XrayApiClient::listOutbounds()
{
    sendRequest("POST", "/v2ray/command/list_outbounds", QJsonObject());
}

void XrayApiClient::sendRequest(const QString& method, const QString& path, const QJsonObject& params)
{
    QString url = buildUrl(path);
    QNetworkRequest request;
    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    if (!m_apiKey.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    }

    if (m_sslEnabled) {
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        
        if (!m_caCertPath.isEmpty()) {
            QList<QSslCertificate> caCerts = QSslCertificate::fromPath(m_caCertPath);
            if (!caCerts.isEmpty()) {
                sslConfig.setCaCertificates(caCerts);
            }
        }
        
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        
        request.setSslConfiguration(sslConfig);
    }

    QNetworkReply* reply = nullptr;
    
    if (method == "POST") {
        QJsonDocument doc(params);
        QByteArray data = doc.toJson(QJsonDocument::Compact);
        reply = m_networkManager->post(request, data);
    } else if (method == "GET") {
        reply = m_networkManager->get(request);
    } else if (method == "DELETE") {
        reply = m_networkManager->deleteResource(request);
    }

    connect(reply, &QNetworkReply::finished, this, &XrayApiClient::onReplyFinished);
}

QString XrayApiClient::buildUrl(const QString& path) const
{
    QString scheme = m_sslEnabled ? "https" : "http";
    return QString("%1://%2:%3%4")
        .arg(scheme)
        .arg(m_host)
        .arg(m_port)
        .arg(path);
}

void XrayApiClient::onReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    handleResponse(reply);
    reply->deleteLater();
}

void XrayApiClient::handleResponse(QNetworkReply* reply)
{
    QNetworkReply::NetworkError error = reply->error();
    
    if (error != QNetworkReply::NoError) {
        QString errorStr = QString("Network error: %1 - %2").arg(error).arg(reply->errorString());
        qWarning() << "XrayApiClient:" << errorStr;
        emit errorOccurred(errorStr);
        return;
    }

    QByteArray data = reply->readAll();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        QString errorStr = QString("JSON parse error: %1").arg(parseError.errorString());
        qWarning() << "XrayApiClient:" << errorStr;
        emit errorOccurred(errorStr);
        return;
    }

    if (!doc.isObject()) {
        emit errorOccurred("Invalid response format");
        return;
    }

    QJsonObject response = doc.object();

    if (response.contains("error")) {
        QJsonObject errorObj = response["error"].toObject();
        QString errorStr = QString("API error: %1").arg(errorObj["message"].toString());
        qWarning() << "XrayApiClient:" << errorStr;
        emit errorOccurred(errorStr);
        return;
    }

    qDebug() << "XrayApiClient: Response received:" << response;
    emit listInboundsResponse(response);
}
