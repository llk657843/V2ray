#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QCheckBox>
#include <QMouseEvent>
#include <QWheelEvent>

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();
    void onLoginResult(bool success);

signals:
    void loginSuccess(const QString &username, const QString &password);
    void closeClicked();
    void forgotPasswordClicked();
    void signUpClicked();

private slots:
    void onSubmitClicked();
    void onEyeClicked();
    void onRememberToggled(bool checked);

private:
    void setupUi();
    void setupConnections();
    void loadRememberedCredentials();
    void stageCredentialsForLoginAttempt();
    void persistStagedCredentialsIfNeeded();
    QString credentialFilePath() const;
    bool writeEncryptedCredentials(const QString &username, const QString &password);
    bool readEncryptedCredentials(QString &username, QString &password);
    void clearCredentialFile();

    // Login Card
    QWidget *m_loginCard;
    QLabel *m_shieldIcon;
    QLabel *m_titleLabel;
    QLabel *m_subtitleLabel;

    // Username field
    QLabel *m_usernameFieldLabel;
    QWidget *m_usernameGroup;
    QLabel *m_usernameIcon;
    QLineEdit *m_usernameEdit;

    // Password field
    QLabel *m_passwordFieldLabel;
    QWidget *m_passwordGroup;
    QLabel *m_passwordIcon;
    QLineEdit *m_passwordEdit;
    QPushButton *m_eyeButton;

    // Actions
    QPushButton *m_forgotButton;
    QCheckBox *m_rememberCheckBox;
    QPushButton *m_submitButton;
    QLabel *m_createAccountHint;
    QPushButton *m_createAccountButton;

    // State
    bool m_passwordVisible;
    bool m_pendingRemember;
    QString m_pendingUsername;
    QString m_pendingPassword;
};

#endif // LOGINWIDGET_H
