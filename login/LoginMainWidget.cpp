#include "LoginMainWidget.h"
#include "LoginWidget.h"
#include "RegisterPage.h"
#include "ResetPwd.h"
#include "VerifyCodePage.h"
#include "AuthClient.h"

#include <QDebug>
#include <QMessageBox>
#include <QThread>

LoginMainWidget::LoginMainWidget(QWidget *parent)
    : QWidget(parent)
    , m_stackedWidget(new QStackedWidget(this))
    , m_loginWidget(nullptr)
    , m_authClient(new AuthClient(this))
    , m_verifyCodePage(nullptr)
    , m_dragging(false)
{
    setupUi();
    
    // 连接 AuthClient 信号
    connect(m_authClient, &AuthClient::loginSuccess, this, &LoginMainWidget::onLoginSuccess);
    connect(m_authClient, &AuthClient::loginFailed, this, &LoginMainWidget::onLoginFailed);
    connect(m_authClient, &AuthClient::registerSuccess, this, &LoginMainWidget::onRegisterSuccess);
    connect(m_authClient, &AuthClient::registerFailed, this, &LoginMainWidget::onRegisterFailed);
    connect(m_authClient, &AuthClient::sendCodeSuccess, this, &LoginMainWidget::onSendCodeSuccess);
    connect(m_authClient, &AuthClient::sendCodeFailed, this, &LoginMainWidget::onSendCodeFailed);
    connect(m_authClient, &AuthClient::resetPasswordSuccess, this, &LoginMainWidget::onResetPasswordSuccess);
    connect(m_authClient, &AuthClient::resetPasswordFailed, this, &LoginMainWidget::onResetPasswordFailed);
}

LoginMainWidget::~LoginMainWidget()
{
}

void LoginMainWidget::setupUi()
{
    setObjectName("LoginMainWidget");
    setWindowTitle(QStringLiteral("XProxy"));
    setFixedSize(1200, 760);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 18, 24, 18);
    mainLayout->setSpacing(0);

    QWidget *header = new QWidget(this);
    header->setObjectName("header");
    header->setFixedHeight(56);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);

    QLabel *logoLabel = new QLabel(QStringLiteral("XProxy"), header);
    logoLabel->setObjectName("logoLabel");

    QPushButton *closeButton = new QPushButton(QStringLiteral("关闭"), header);
    closeButton->setObjectName("closeButton");
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFocusPolicy(Qt::NoFocus);
    closeButton->setFixedHeight(32);
    closeButton->setMinimumWidth(56);

    headerLayout->addWidget(logoLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);

    QWidget *contentArea = new QWidget(this);
    contentArea->setObjectName("contentArea");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addStretch();
    contentLayout->addWidget(m_stackedWidget, 0, Qt::AlignCenter);
    contentLayout->addStretch();

    // 登录页面
    m_loginWidget = new LoginWidget(contentArea);
    QWidget *loginPage = new QWidget(contentArea);
    loginPage->setObjectName("loginPage");
    QVBoxLayout *loginPageLayout = new QVBoxLayout(loginPage);
    loginPageLayout->setContentsMargins(0, 0, 0, 0);
    loginPageLayout->addWidget(m_loginWidget, 0, Qt::AlignCenter);

    m_stackedWidget->addWidget(loginPage);
    m_pages["login"] = loginPage;
    m_stackedWidget->setCurrentWidget(loginPage);

    // 验证码页面
    m_verifyCodePage = new VerifyCodePage(contentArea);
    QWidget *verifyPage = new QWidget(contentArea);
    verifyPage->setObjectName("verifyPage");
    QVBoxLayout *verifyPageLayout = new QVBoxLayout(verifyPage);
    verifyPageLayout->setContentsMargins(0, 0, 0, 0);
    verifyPageLayout->addWidget(m_verifyCodePage, 0, Qt::AlignCenter);

    m_stackedWidget->addWidget(verifyPage);
    m_pages["verify"] = verifyPage;

    connect(m_verifyCodePage, &VerifyCodePage::verifyRequested, this, &LoginMainWidget::onVerifyCodeSubmitted);
    connect(m_verifyCodePage, &VerifyCodePage::resendRequested, this, [this]() {
        if (m_resetEmail.isEmpty()) {
            QMessageBox::warning(this, tr("无法重发"), tr("请先输入邮箱并发送验证码。"));
            return;
        }
        m_authClient->sendResetCode(m_resetEmail);
    });

    // 重置密码页面
    ResetPwd *resetPwdPage = new ResetPwd(contentArea);
    QWidget *resetPage = new QWidget(contentArea);
    resetPage->setObjectName("resetPwdPage");
    QVBoxLayout *resetPageLayout = new QVBoxLayout(resetPage);
    resetPageLayout->setContentsMargins(0, 0, 0, 0);
    resetPageLayout->addWidget(resetPwdPage, 0, Qt::AlignCenter);

    m_stackedWidget->addWidget(resetPage);
    m_pages["resetPwd"] = resetPage;

    connect(resetPwdPage, &ResetPwd::sendResetLinkRequested, this,
        [this](const QString &identifier) {
            m_resetEmail = identifier;
            m_authClient->sendResetCode(identifier);
        });
    connect(resetPwdPage, &ResetPwd::backToLoginRequested, this, [this]() {
        switchPage(QStringLiteral("login"));
    });

    // 注册页面
    RegisterPage *registerPageWidget = new RegisterPage(contentArea);
    QWidget *registerPage = new QWidget(contentArea);
    registerPage->setObjectName("registerPage");
    QVBoxLayout *registerPageLayout = new QVBoxLayout(registerPage);
    registerPageLayout->setContentsMargins(0, 0, 0, 0);
    registerPageLayout->addWidget(registerPageWidget, 0, Qt::AlignCenter);

    m_stackedWidget->addWidget(registerPage);
    m_pages["register"] = registerPage;

    connect(registerPageWidget, &RegisterPage::backToLoginRequested, this, [this]() {
        switchPage(QStringLiteral("login"));
    });
    connect(registerPageWidget, &RegisterPage::accountInitializeRequested, this,
        &LoginMainWidget::onAccountInitializeRequested);

    // 底部
    QWidget *footer = new QWidget(this);
    footer->setObjectName("footer");
    footer->setFixedHeight(34);
    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(12);

    QLabel *copyrightLabel = new QLabel(
        "© 2024 KINETIC PROXY SYSTEMS. ALL RIGHTS RESERVED.",
        footer);
    copyrightLabel->setObjectName("copyrightLabel");

    QPushButton *privacyButton = new QPushButton(QStringLiteral("隐私政策"), footer);
    privacyButton->setObjectName("footerPrivacyButton");
    privacyButton->setCursor(Qt::PointingHandCursor);
    privacyButton->setFocusPolicy(Qt::NoFocus);

    QPushButton *termsButton = new QPushButton(QStringLiteral("服务条款"), footer);
    termsButton->setObjectName("footerTermsButton");
    termsButton->setCursor(Qt::PointingHandCursor);
    termsButton->setFocusPolicy(Qt::NoFocus);

    footerLayout->addWidget(copyrightLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(privacyButton);
    footerLayout->addWidget(termsButton);

    mainLayout->addWidget(header);
    mainLayout->addWidget(contentArea, 1);
    mainLayout->addWidget(footer);

    connect(closeButton, &QPushButton::clicked, this, &LoginMainWidget::onCloseClicked);
    
    // 登录按钮点击 - 调用 API
    connect(m_loginWidget, &LoginWidget::loginSuccess, this, [this](const QString &email, const QString &password) {
        m_authClient->login(email, password);
    });
    
    connect(m_loginWidget, &LoginWidget::closeClicked, this, &LoginMainWidget::onCloseClicked);
    connect(m_loginWidget, &LoginWidget::forgotPasswordClicked, this, [this]() {
        switchPage(QStringLiteral("resetPwd"));
    });
    connect(m_loginWidget, &LoginWidget::signUpClicked, this, [this]() {
        switchPage(QStringLiteral("register"));
    });
}

void LoginMainWidget::addPage(const QString &name, QWidget *widget)
{
    m_stackedWidget->addWidget(widget);
    m_pages[name] = widget;
}

void LoginMainWidget::switchPage(const QString &name)
{
    if (m_pages.contains(name)) {
        m_stackedWidget->setCurrentWidget(m_pages[name]);
    } else {
        qWarning() << "Page not found:" << name;
    }
}

void LoginMainWidget::onCloseClicked()
{
    emit loginClose();
    close();
}

void LoginMainWidget::onVerifyCodeSubmitted(const QString &code, const QString &newPassword)
{
    if (m_resetEmail.isEmpty()) {
        QMessageBox::warning(this, tr("重置失败"), tr("缺少邮箱信息，请重新开始重置流程。"));
        switchPage(QStringLiteral("resetPwd"));
        return;
    }

    emit verifyCodeSubmitted(code);
    m_authClient->resetPassword(m_resetEmail, newPassword, code);
}

void LoginMainWidget::onAccountInitializeRequested(const QString &email, const QString &password)
{
    // 调用注册 API
    m_authClient->registerUser(email, password);
}

// ============ AuthClient 回调 ============

void LoginMainWidget::onLoginSuccess(const QString &token, const QString &expireAt, const QJsonObject &user)
{
    Q_UNUSED(expireAt);
    if (m_loginWidget != nullptr) {
        m_loginWidget->onLoginResult(true);
    }
    emit loginSuccess(token, user);
}

void LoginMainWidget::onLoginFailed(const QString &message)
{
    if (m_loginWidget != nullptr) {
        m_loginWidget->onLoginResult(false);
    }
    QMessageBox::warning(this, tr("登录失败"), message);
}

void LoginMainWidget::onRegisterSuccess(const QString &userId, const QString &createdAt)
{
    Q_UNUSED(userId);
    Q_UNUSED(createdAt);
    QMessageBox::information(this, tr("注册成功"), tr("账号创建成功，请登录。"));
    switchPage(QStringLiteral("login"));
}

void LoginMainWidget::onRegisterFailed(const QString &message)
{
    QMessageBox::warning(this, tr("注册失败"), message);
}

void LoginMainWidget::onSendCodeSuccess()
{
    m_verifyCodePage->clearCode();
    m_verifyCodePage->clearPasswordFields();
    switchPage(QStringLiteral("verify"));
    QMessageBox::information(this, tr("验证码已发送"), tr("验证码已发送到您的邮箱，请注意查收。"));
}

void LoginMainWidget::onSendCodeFailed(const QString &message)
{
    QMessageBox::warning(this, tr("发送失败"), message);
}

void LoginMainWidget::onResetPasswordSuccess()
{
    QMessageBox::information(this, tr("重置成功"), tr("密码已重置，请使用新密码登录。"));
    m_resetEmail.clear();
    switchPage(QStringLiteral("login"));
}

void LoginMainWidget::onResetPasswordFailed(const QString &message)
{
    QMessageBox::warning(this, tr("重置失败"), message);
}

// ============ 窗口拖动 ============

void LoginMainWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void LoginMainWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void LoginMainWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}
