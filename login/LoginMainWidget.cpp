#include "LoginMainWidget.h"
#include "LoginWidget.h"

#include <QDebug>

LoginMainWidget::LoginMainWidget(QWidget *parent)
    : QWidget(parent)
    , m_stackedWidget(new QStackedWidget(this))
    , m_dragging(false)
{
    setupUi();
}

LoginMainWidget::~LoginMainWidget()
{
}

void LoginMainWidget::setupUi()
{
    setObjectName("LoginMainWidget");
    setWindowTitle("Kinetic Login");
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

    QLabel *logoLabel = new QLabel("KINETIC", header);
    logoLabel->setObjectName("logoLabel");

    QPushButton *closeButton = new QPushButton("x", header);
    closeButton->setObjectName("closeButton");
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFocusPolicy(Qt::NoFocus);
    closeButton->setFixedSize(32, 32);

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

    LoginWidget *loginWidget = new LoginWidget(contentArea);
    QWidget *loginPage = new QWidget(contentArea);
    loginPage->setObjectName("loginPage");
    QVBoxLayout *loginPageLayout = new QVBoxLayout(loginPage);
    loginPageLayout->setContentsMargins(0, 0, 0, 0);
    loginPageLayout->addWidget(loginWidget, 0, Qt::AlignCenter);

    m_stackedWidget->addWidget(loginPage);
    m_pages["login"] = loginPage;
    m_stackedWidget->setCurrentWidget(loginPage);

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

    QPushButton *privacyButton = new QPushButton("PRIVACY POLICY", footer);
    privacyButton->setObjectName("footerLinkButton");
    privacyButton->setCursor(Qt::PointingHandCursor);
    privacyButton->setFocusPolicy(Qt::NoFocus);

    QPushButton *termsButton = new QPushButton("TERMS OF SERVICE", footer);
    termsButton->setObjectName("footerLinkButton");
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
    connect(loginWidget, &LoginWidget::loginSuccess, this, [this](const QString &, const QString &) {
        emit loginSuccess();
    });
    connect(loginWidget, &LoginWidget::closeClicked, this, &LoginMainWidget::onCloseClicked);
    connect(loginWidget, &LoginWidget::forgotPasswordClicked, this, []() {
        qDebug() << "Forgot password clicked";
    });
    connect(loginWidget, &LoginWidget::signUpClicked, this, []() {
        qDebug() << "Sign up clicked";
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
