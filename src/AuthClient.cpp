#include "AuthClient.h"
#include <curl/curl.h>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QStringList>
#include <QUrl>
#include "Logger.h"

AuthClient::AuthClient(QObject *parent)
    : QObject(parent)
    , m_serverUrl("http://47.79.33.96:80")
    , m_curl(nullptr)
{
    loadServerUrlFromConfig();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_curl = curl_easy_init();
}

AuthClient::~AuthClient()
{
    if (m_curl) {
        curl_easy_cleanup(static_cast<CURL*>(m_curl));
    }
    curl_global_cleanup();
}

void AuthClient::setServerUrl(const QString &url)
{
    m_serverUrl = normalizeServerUrl(url);
    Logger::instance()->info(QStringLiteral("AuthClient serverUrl set: %1").arg(m_serverUrl));
}

QString AuthClient::serverUrl() const
{
    return m_serverUrl;
}

QString AuthClient::normalizeServerUrl(const QString &url)
{
    const QString trimmed = url.trimmed();
    if (trimmed.isEmpty()) return trimmed;

    QUrl u(trimmed);
    if (!u.isValid() || u.scheme().isEmpty()) {
        u = QUrl(QStringLiteral("http://") + trimmed);
    }

    // If no explicit port specified, default to 80.
    if (u.isValid() && u.port() == -1) {
        u.setPort(80);
    }

    return u.toString(QUrl::RemoveFragment | QUrl::StripTrailingSlash);
}

size_t AuthClient::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    auto *str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

void AuthClient::loadServerUrlFromConfig()
{
    const QStringList configCandidates = {
        QCoreApplication::applicationDirPath() + QStringLiteral("/auth_client.json"),
        QDir::currentPath() + QStringLiteral("/auth_client.json")
    };

    for (const QString &configPath : configCandidates) {
        QFile file(configPath);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            Logger::instance()->warning(
                QStringLiteral("AuthClient config parse error: %1 path=%2")
                    .arg(parseError.errorString(), configPath));
            continue;
        }

        const QString configuredUrl = doc.object().value(QStringLiteral("serverUrl")).toString().trimmed();
        if (!configuredUrl.isEmpty()) {
            m_serverUrl = normalizeServerUrl(configuredUrl);
            Logger::instance()->info(QStringLiteral("AuthClient serverUrl loaded from config: %1").arg(m_serverUrl));
            return;
        }
    }
}

bool AuthClient::postJson(const QString &path,
                          const QJsonObject &body,
                          QJsonObject &response,
                          QString *errorMessage,
                          long *httpCodeOut)
{
    if (!m_curl) {
        if (errorMessage) *errorMessage = QStringLiteral("CURL 未初始化");
        Logger::instance()->error(QStringLiteral("AuthClient postJson failed: curl not initialized"));
        return false;
    }

    const QString url = m_serverUrl + path;
    QJsonDocument doc(body);
    const QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    CURL *curl = static_cast<CURL*>(m_curl);
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8().constData());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.constData());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(jsonData.size()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    std::string responseStr;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

    Logger::instance()->info(
        QStringLiteral("AuthClient POST %1 body=%2")
            .arg(url, QString::fromUtf8(jsonData)));

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        const QString err = QStringLiteral("CURL error (%1): %2")
                                .arg(static_cast<int>(res))
                                .arg(QString::fromUtf8(curl_easy_strerror(res)));
        if (errorMessage) *errorMessage = err;
        Logger::instance()->error(QStringLiteral("AuthClient request failed: %1 url=%2").arg(err, url));
        return false;
    }

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    if (httpCodeOut) *httpCodeOut = httpCode;

    const QString responseQt = QString::fromUtf8(QByteArray::fromStdString(responseStr));
    Logger::instance()->info(
        QStringLiteral("AuthClient response url=%1 http=%2 body=%3")
            .arg(url)
            .arg(httpCode)
            .arg(responseQt));

    if (httpCode < 200 || httpCode >= 300) {
        const QString err = QStringLiteral("HTTP %1: %2").arg(httpCode).arg(responseQt.isEmpty() ? QStringLiteral("<empty>") : responseQt);
        if (errorMessage) *errorMessage = err;
        Logger::instance()->warning(QStringLiteral("AuthClient non-2xx response: %1 url=%2").arg(err, url));
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument responseDoc = QJsonDocument::fromJson(
        QByteArray::fromStdString(responseStr), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        const QString err = QStringLiteral("JSON parse error: %1").arg(parseError.errorString());
        if (errorMessage) *errorMessage = err;
        Logger::instance()->error(QStringLiteral("AuthClient parse failed: %1 url=%2 body=%3").arg(err, url, responseQt));
        return false;
    }

    response = responseDoc.object();
    return true;
}

void AuthClient::registerUser(const QString &email, const QString &password)
{
    QJsonObject body;
    body["email"] = email;
    body["password"] = password;

    QJsonObject response;
    QString err;
    if (!postJson("/api/v1/auth/register", body, response, &err)) {
        emit registerFailed(err.isEmpty() ? QStringLiteral("网络请求失败") : err);
        return;
    }

    int code = response["code"].toInt();
    QString message = response["message"].toString();

    if (code == 200) {
        QJsonObject data = response["data"].toObject();
        QString userId = data["userId"].toString();
        QString createdAt = data["createdAt"].toString();
        emit registerSuccess(userId, createdAt);
    } else {
        emit registerFailed(message);
    }
}

void AuthClient::login(const QString &email, const QString &password)
{
    QJsonObject body;
    body["email"] = email;
    body["password"] = password;

    QJsonObject response;
    QString err;
    if (!postJson("/api/v1/auth/login", body, response, &err)) {
        emit loginFailed(err.isEmpty() ? QStringLiteral("网络请求失败") : err);
        return;
    }

    int code = response["code"].toInt();
    QString message = response["message"].toString();

    if (code == 200) {
        QJsonObject data = response["data"].toObject();
        QString token = data["token"].toString();
        QString expireAt = data["expire_at"].toString();
        QJsonObject user = data["user"].toObject();
        emit loginSuccess(token, expireAt, user);
    } else {
        emit loginFailed(message);
    }
}

void AuthClient::sendResetCode(const QString &email)
{
    QJsonObject body;
    body["email"] = email;

    QJsonObject response;
    QString err;
    if (!postJson("/api/v1/auth/password/send-code", body, response, &err)) {
        emit sendCodeFailed(err.isEmpty() ? QStringLiteral("网络请求失败") : err);
        return;
    }

    int code = response["code"].toInt();
    QString message = response["message"].toString();

    if (code == 200) {
        emit sendCodeSuccess();
    } else {
        emit sendCodeFailed(message);
    }
}

void AuthClient::resetPassword(const QString &email, const QString &newPassword, const QString &code)
{
    QJsonObject body;
    body["email"] = email;
    body["password"] = newPassword;
    body["code"] = code;

    QJsonObject response;
    QString err;
    if (!postJson("/api/auth/password/reset", body, response, &err)) {
        emit resetPasswordFailed(err.isEmpty() ? QStringLiteral("网络请求失败") : err);
        return;
    }

    int codeResp = response["code"].toInt();
    QString message = response["message"].toString();

    if (codeResp == 200) {
        emit resetPasswordSuccess();
    } else {
        emit resetPasswordFailed(message);
    }
}
