#include "LoginMainWidget.h"
#include "LoginWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
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
    // Main window settings
    setFixedSize(460, 640);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_StyledBackground);

    // Set dark background color
    setStyleSheet("background-color: #0b0c10;");

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Login page (page 0) - contains LoginWidget
    QWidget *loginPage = new QWidget(this);
    loginPage->setObjectName("loginPage");
    QHBoxLayout *loginPageLayout = new QHBoxLayout(loginPage);
    loginPageLayout->setContentsMargins(40, 40, 40, 40);

    LoginWidget *loginWidget = new LoginWidget(loginPage);
    loginPageLayout->addWidget(loginWidget);
    m_stackedWidget->addWidget(loginPage);
    m_pages["login"] = loginPage;

    // Connect LoginWidget signals
    connect(loginWidget, &LoginWidget::loginSuccess, this, [this]() {
        emit loginSuccess();
    });
    connect(loginWidget, &LoginWidget::closeClicked, this, [this]() {
        emit loginClose();
        close();
    });
    connect(loginWidget, &LoginWidget::forgotPasswordClicked, this, [this]() {
        qDebug() << "Forgot password clicked - switch to reset verify page";
        // Future: switchPage("resetVerify");
    });
    connect(loginWidget, &LoginWidget::signUpClicked, this, [this]() {
        qDebug() << "Sign up clicked - switch to create account page";
        // Future: switchPage("createAccount");
    });

    // Placeholder pages (for future expansion)
    QWidget *resetVerifyPage = new QWidget(this);
    resetVerifyPage->setObjectName("resetVerifyPage");
    QVBoxLayout *resetLayout = new QVBoxLayout(resetVerifyPage);
    resetLayout->addWidget(new QLabel("Reset verify page (placeholder)", resetVerifyPage));
    resetLayout->addStretch();
    m_stackedWidget->addWidget(resetVerifyPage);
    m_pages["resetVerify"] = resetVerifyPage;

    QWidget *createAccountPage = new QWidget(this);
    createAccountPage->setObjectName("createAccountPage");
    QVBoxLayout *createLayout = new QVBoxLayout(createAccountPage);
    createLayout->addWidget(new QLabel("Create account page (placeholder)", createAccountPage));
    createLayout->addStretch();
    m_stackedWidget->addWidget(createAccountPage);
    m_pages["createAccount"] = createAccountPage;

    mainLayout->addWidget(m_stackedWidget);
    m_stackedWidget->setCurrentWidget(loginPage);
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
        m_dragPosition = mapToGlobal(event->pos()) - frameGeometry().topLeft();
        event->accept();
    }
}

void LoginMainWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && event->buttons() & Qt::LeftButton) {
        move(mapToGlobal(event->pos()) - m_dragPosition);
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
