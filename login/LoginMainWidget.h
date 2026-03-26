#ifndef LoginMainWidget_H
#define LoginMainWidget_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QMouseEvent>
#include <QMap>
#include <QStackedWidget>
#include <QJsonObject>

class LoginWidget;
class AuthClient;
class VerifyCodePage;

class LoginMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginMainWidget(QWidget *parent = nullptr);
    ~LoginMainWidget();

    void addPage(const QString &name, QWidget *widget);
    void switchPage(const QString &name);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void loginSuccess(const QString &token, const QJsonObject &user);
    void loginClose();
    void verifyCodeSubmitted(const QString &code);
    void resendVerificationRequested();
    void resetLinkRequested(const QString &identifier);
    void accountInitializeRequested(const QString &email, const QString &password);

private slots:
    void onCloseClicked();
    void onVerifyCodeSubmitted(const QString &code, const QString &newPassword);
    void onAccountInitializeRequested(const QString &email, const QString &password);
    
    // AuthClient 回调
    void onLoginSuccess(const QString &token, const QString &expireAt, const QJsonObject &user);
    void onLoginFailed(const QString &message);
    void onRegisterSuccess(const QString &userId, const QString &createdAt);
    void onRegisterFailed(const QString &message);
    void onSendCodeSuccess();
    void onSendCodeFailed(const QString &message);
    void onResetPasswordSuccess();
    void onResetPasswordFailed(const QString &message);

private:
    void setupUi();

    // Content
    QStackedWidget *m_stackedWidget;
    LoginWidget *m_loginWidget;
    AuthClient *m_authClient;
    VerifyCodePage *m_verifyCodePage;
    QString m_resetEmail;  // 用于重置密码流程

    // Page management
    QMap<QString, QWidget *> m_pages;

    // Window dragging
    bool m_dragging;
    QPoint m_dragPosition;
};

#endif // LoginMainWidget_H
