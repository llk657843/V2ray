#include "LoginWidget.h"

#include <QFrame>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , m_passwordVisible(false)
{
    setupUi();
    setupConnections();
}

LoginWidget::~LoginWidget()
{
}

void LoginWidget::setupUi()
{
    setObjectName("loginCardRoot");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_loginCard = new QWidget(this);
    m_loginCard->setObjectName("loginCard");
    m_loginCard->setFixedSize(470, 560);

    QVBoxLayout *cardLayout = new QVBoxLayout(m_loginCard);
    cardLayout->setContentsMargins(36, 34, 36, 28);
    cardLayout->setSpacing(0);

    QWidget *shieldContainer = new QWidget(m_loginCard);
    shieldContainer->setObjectName("shieldIconContainer");
    shieldContainer->setFixedSize(42, 42);
    QHBoxLayout *shieldLayout = new QHBoxLayout(shieldContainer);
    shieldLayout->setContentsMargins(0, 0, 0, 0);
    shieldLayout->setAlignment(Qt::AlignCenter);

    m_shieldIcon = new QLabel("o", shieldContainer);
    m_shieldIcon->setObjectName("shieldIcon");
    m_shieldIcon->setAlignment(Qt::AlignCenter);
    shieldLayout->addWidget(m_shieldIcon);

    m_titleLabel = new QLabel("Welcome Back", m_loginCard);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel("Access your obsidian-class proxy network.", m_loginCard);
    m_subtitleLabel->setObjectName("subtitleLabel");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    m_usernameFieldLabel = new QLabel("ACCOUNT IDENTIFIER", m_loginCard);
    m_usernameFieldLabel->setObjectName("usernameFieldLabel");

    m_usernameGroup = new QFrame(m_loginCard);
    m_usernameGroup->setObjectName("usernameGroup");
    m_usernameGroup->setFixedHeight(44);

    QHBoxLayout *usernameLayout = new QHBoxLayout(m_usernameGroup);
    usernameLayout->setContentsMargins(14, 0, 14, 0);
    usernameLayout->setSpacing(8);

    m_usernameIcon = new QLabel("@", m_usernameGroup);
    m_usernameIcon->setObjectName("usernameIcon");
    m_usernameIcon->setFixedSize(18, 18);
    m_usernameIcon->setAlignment(Qt::AlignCenter);
    usernameLayout->addWidget(m_usernameIcon);

    m_usernameEdit = new QLineEdit(m_usernameGroup);
    m_usernameEdit->setObjectName("usernameEdit");
    m_usernameEdit->setPlaceholderText("email@kinetic-proxy.com");
    m_usernameEdit->setFrame(false);
    usernameLayout->addWidget(m_usernameEdit);

    QHBoxLayout *passwordHeaderLayout = new QHBoxLayout();
    passwordHeaderLayout->setContentsMargins(0, 0, 0, 0);
    passwordHeaderLayout->setSpacing(0);

    m_passwordFieldLabel = new QLabel("SECURITY KEY", m_loginCard);
    m_passwordFieldLabel->setObjectName("passwordFieldLabel");
    passwordHeaderLayout->addWidget(m_passwordFieldLabel);
    passwordHeaderLayout->addStretch();

    m_forgotButton = new QPushButton("FORGOT?", m_loginCard);
    m_forgotButton->setObjectName("forgotButton");
    m_forgotButton->setCursor(Qt::PointingHandCursor);
    m_forgotButton->setFocusPolicy(Qt::NoFocus);
    passwordHeaderLayout->addWidget(m_forgotButton);

    m_passwordGroup = new QFrame(m_loginCard);
    m_passwordGroup->setObjectName("passwordGroup");
    m_passwordGroup->setFixedHeight(44);

    QHBoxLayout *passwordLayout = new QHBoxLayout(m_passwordGroup);
    passwordLayout->setContentsMargins(14, 0, 14, 0);
    passwordLayout->setSpacing(8);

    m_passwordIcon = new QLabel("o", m_passwordGroup);
    m_passwordIcon->setObjectName("passwordIcon");
    m_passwordIcon->setFixedSize(18, 18);
    m_passwordIcon->setAlignment(Qt::AlignCenter);
    passwordLayout->addWidget(m_passwordIcon);

    m_passwordEdit = new QLineEdit(m_passwordGroup);
    m_passwordEdit->setObjectName("passwordEdit");
    m_passwordEdit->setPlaceholderText("............");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setFrame(false);
    passwordLayout->addWidget(m_passwordEdit);

    m_eyeButton = new QPushButton("o", m_passwordGroup);
    m_eyeButton->setObjectName("eyeButton");
    m_eyeButton->setFixedSize(22, 22);
    m_eyeButton->setCursor(Qt::PointingHandCursor);
    m_eyeButton->setFocusPolicy(Qt::NoFocus);
    passwordLayout->addWidget(m_eyeButton);

    m_submitButton = new QPushButton("Initialize Connection   >", m_loginCard);
    m_submitButton->setObjectName("submitButton");
    m_submitButton->setFixedHeight(46);
    m_submitButton->setCursor(Qt::PointingHandCursor);
    m_submitButton->setFocusPolicy(Qt::NoFocus);

    QFrame *separator = new QFrame(m_loginCard);
    separator->setObjectName("cardSeparator");
    separator->setFrameShape(QFrame::HLine);
    separator->setFixedHeight(1);

    QHBoxLayout *createAccountRow = new QHBoxLayout();
    createAccountRow->setContentsMargins(0, 0, 0, 0);
    createAccountRow->setSpacing(0);
    m_createAccountHint = new QLabel(tr("New to the network? "), m_loginCard);
    m_createAccountHint->setObjectName("loginCreateAccountHint");
    m_createAccountButton = new QPushButton(tr("Create Account"), m_loginCard);
    m_createAccountButton->setObjectName("loginCreateAccountLink");
    m_createAccountButton->setFlat(true);
    m_createAccountButton->setCursor(Qt::PointingHandCursor);
    m_createAccountButton->setFocusPolicy(Qt::NoFocus);
    createAccountRow->addStretch();
    createAccountRow->addWidget(m_createAccountHint);
    createAccountRow->addWidget(m_createAccountButton);
    createAccountRow->addStretch();

    cardLayout->addSpacing(6);
    cardLayout->addWidget(shieldContainer, 0, Qt::AlignCenter);
    cardLayout->addSpacing(14);
    cardLayout->addWidget(m_titleLabel);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(m_subtitleLabel);
    cardLayout->addSpacing(24);
    cardLayout->addWidget(m_usernameFieldLabel);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(m_usernameGroup);
    cardLayout->addSpacing(14);
    cardLayout->addLayout(passwordHeaderLayout);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(m_passwordGroup);
    cardLayout->addSpacing(20);
    cardLayout->addWidget(m_submitButton);
    cardLayout->addSpacing(22);
    cardLayout->addWidget(separator);
    cardLayout->addSpacing(18);
    cardLayout->addLayout(createAccountRow);
    cardLayout->addStretch();

    mainLayout->addWidget(m_loginCard, 0, Qt::AlignCenter);
}

void LoginWidget::setupConnections()
{
    connect(m_submitButton, &QPushButton::clicked, this, &LoginWidget::onSubmitClicked);
    connect(m_eyeButton, &QPushButton::clicked, this, &LoginWidget::onEyeClicked);
    connect(m_forgotButton, &QPushButton::clicked, this, &LoginWidget::forgotPasswordClicked);
    connect(m_createAccountButton, &QPushButton::clicked, this, [this]() {
        emit signUpClicked();
    });
}

void LoginWidget::onSubmitClicked()
{
    emit loginSuccess(m_usernameEdit->text(), m_passwordEdit->text());
}

void LoginWidget::onEyeClicked()
{
    m_passwordVisible = !m_passwordVisible;
    m_passwordEdit->setEchoMode(m_passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
    m_eyeButton->setText(m_passwordVisible ? "-" : "o");
}
