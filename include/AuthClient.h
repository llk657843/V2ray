#ifndef AUTHCLIENT_H
#define AUTHCLIENT_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <functional>
#include <QObject>

class AuthClient : public QObject
{
    Q_OBJECT

public:
    explicit AuthClient(QObject *parent = nullptr);
    ~AuthClient();

    // 设置服务器地址
    void setServerUrl(const QString &url);
    QString serverUrl() const;

    // 注册
    // POST /api/v1/auth/register
    // Body: { "email": "...", "password": "..." }
    void registerUser(const QString &email, const QString &password);

    // 登录
    // POST /api/v1/auth/login
    // Body: { "email": "...", "password": "..." }
    void login(const QString &email, const QString &password);

    // 发送验证码（重置密码用）
    // POST /api/v1/auth/password/send-code
    // Body: { "email": "..." }
    void sendResetCode(const QString &email);

    // 重置密码
    // POST /api/auth/password/reset
    // Body: { "email": "...", "password": "...", "code": "..." }
    void resetPassword(const QString &email, const QString &newPassword, const QString &code);

signals:
    // 注册结果
    void registerSuccess(const QString &userId, const QString &createdAt);
    void registerFailed(const QString &message);

    // 登录结果
    void loginSuccess(const QString &token, const QString &expireAt, const QJsonObject &user);
    void loginFailed(const QString &message);

    // 发送验证码结果
    void sendCodeSuccess();
    void sendCodeFailed(const QString &message);

    // 重置密码结果
    void resetPasswordSuccess();
    void resetPasswordFailed(const QString &message);

private:
    QString m_serverUrl;
    void *m_curl;  // CURL*

    static QString normalizeServerUrl(const QString &url);
    void loadServerUrlFromConfig();
    bool postJson(const QString &path,
                  const QJsonObject &body,
                  QJsonObject &response,
                  QString *errorMessage = nullptr,
                  long *httpCodeOut = nullptr);
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
};

#endif // AUTHCLIENT_H
