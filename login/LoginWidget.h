#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QMouseEvent>
#include <QWheelEvent>

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

signals:
    void loginSuccess(const QString &username, const QString &password);
    void closeClicked();
    void forgotPasswordClicked();
    void signUpClicked();

private slots:
    void onSubmitClicked();
    void onEyeClicked();
    void onCloseClicked();

private:
    void setupUi();
    void setupConnections();

    // Header
    QLabel *m_logoLabel;
    QPushButton *m_closeButton;

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
    QPushButton *m_submitButton;
    QLabel *m_createAccountLabel;

    // Footer
    QLabel *m_copyrightLabel;
    QPushButton *m_termsButton;
    QPushButton *m_privacyButton;

    // State
    bool m_passwordVisible;
};

#endif // LOGINWIDGET_H
