#include "LoginWidget.h"
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

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
    // ---------- Main Layout ----------
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---------- Header (48px) ----------
    QWidget *header = new QWidget(this);
    header->setObjectName("header");
    header->setFixedHeight(48);

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    m_logoLabel = new QLabel("KINETIC", header);
    m_logoLabel->setObjectName("logoLabel");
    m_logoLabel->setCursor(Qt::PointingHandCursor);

    m_closeButton = new QPushButton("✕", header);
    m_closeButton->setObjectName("closeButton");
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setFocusPolicy(Qt::NoFocus);

    headerLayout->addWidget(m_logoLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_closeButton);

    // ---------- Login Card (Center) ----------
    m_loginCard = new QWidget(this);
    m_loginCard->setObjectName("loginCard");
    m_loginCard->setFixedWidth(380);

    // Shadow effect for card
    QGraphicsDropShadowEffect *cardShadow = new QGraphicsDropShadowEffect(m_loginCard);
    cardShadow->setBlurRadius(40);
    cardShadow->setColor(QColor(0, 0, 0, 120));
    cardShadow->setOffset(0, 8);
    m_loginCard->setGraphicsEffect(cardShadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(m_loginCard);
    cardLayout->setContentsMargins(32, 32, 32, 32);
    cardLayout->setSpacing(0);

    // --- Shield Icon ---
    m_shieldIcon = new QLabel("🛡", m_loginCard);
    m_shieldIcon->setObjectName("shieldIcon");
    m_shieldIcon->setAlignment(Qt::AlignCenter);
    m_shieldIcon->setFixedSize(36, 36);

    // --- Title & Subtitle ---
    m_titleLabel = new QLabel("Welcome back", m_loginCard);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel("Sign in to your account", m_loginCard);
    m_subtitleLabel->setObjectName("subtitleLabel");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    // --- Username Field ---
    m_usernameFieldLabel = new QLabel("USERNAME", m_loginCard);
    m_usernameFieldLabel->setObjectName("usernameFieldLabel");
    m_usernameFieldLabel->setProperty("class", "fieldLabel");

    m_usernameGroup = new QWidget(m_loginCard);
    m_usernameGroup->setObjectName("usernameGroup");
    m_usernameGroup->setProperty("class", "inputGroup");

    QHBoxLayout *usernameLayout = new QHBoxLayout(m_usernameGroup);
    usernameLayout->setContentsMargins(12, 0, 12, 0);
    usernameLayout->setSpacing(10);

    m_usernameIcon = new QLabel("👤", m_usernameGroup);
    m_usernameIcon->setObjectName("usernameIcon");
    m_usernameIcon->setProperty("class", "inputIcon");

    m_usernameEdit = new QLineEdit(m_usernameGroup);
    m_usernameEdit->setObjectName("usernameEdit");
    m_usernameEdit->setPlaceholderText("Username or email");
    m_usernameEdit->setFocusPolicy(Qt::StrongFocus);

    usernameLayout->addWidget(m_usernameIcon);
    usernameLayout->addWidget(m_usernameEdit);

    // --- Password Field ---
    m_passwordFieldLabel = new QLabel("PASSWORD", m_loginCard);
    m_passwordFieldLabel->setObjectName("passwordFieldLabel");
    m_passwordFieldLabel->setProperty("class", "fieldLabel");

    m_passwordGroup = new QWidget(m_loginCard);
    m_passwordGroup->setObjectName("passwordGroup");
    m_passwordGroup->setProperty("class", "inputGroup");

    QHBoxLayout *passwordLayout = new QHBoxLayout(m_passwordGroup);
    passwordLayout->setContentsMargins(12, 0, 12, 0);
    passwordLayout->setSpacing(10);

    m_passwordIcon = new QLabel("🔒", m_passwordGroup);
    m_passwordIcon->setObjectName("passwordIcon");
    m_passwordIcon->setProperty("class", "inputIcon");

    m_passwordEdit = new QLineEdit(m_passwordGroup);
    m_passwordEdit->setObjectName("passwordEdit");
    m_passwordEdit->setPlaceholderText("Password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);

    m_eyeButton = new QPushButton("👁", m_passwordGroup);
    m_eyeButton->setObjectName("eyeButton");
    m_eyeButton->setCursor(Qt::PointingHandCursor);
    m_eyeButton->setFocusPolicy(Qt::NoFocus);
    m_eyeButton->setFixedSize(24, 24);

    passwordLayout->addWidget(m_passwordIcon);
    passwordLayout->addWidget(m_passwordEdit);
    passwordLayout->addWidget(m_eyeButton);

    // --- Forgot Button ---
    m_forgotButton = new QPushButton("Forgot password?", m_loginCard);
    m_forgotButton->setObjectName("forgotButton");
    m_forgotButton->setCursor(Qt::PointingHandCursor);
    m_forgotButton->setFocusPolicy(Qt::NoFocus);

    // --- Submit Button ---
    m_submitButton = new QPushButton("Sign in", m_loginCard);
    m_submitButton->setObjectName("submitButton");
    m_submitButton->setCursor(Qt::PointingHandCursor);
    m_submitButton->setFocusPolicy(Qt::NoFocus);
    m_submitButton->setFixedHeight(42);

    // --- Divider & Sign up link ---
    QLabel *divider = new QLabel(m_loginCard);
    divider->setObjectName("divider");

    m_createAccountLabel = new QLabel("Don't have an account? Sign up", m_loginCard);
    m_createAccountLabel->setObjectName("createAccountLabel");
    m_createAccountLabel->setAlignment(Qt::AlignCenter);
    m_createAccountLabel->setCursor(Qt::PointingHandCursor);

    // ---------- Footer ----------
    QWidget *footer = new QWidget(this);
    footer->setObjectName("footer");
    footer->setFixedHeight(40);

    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(24, 0, 24, 0);
    footerLayout->setSpacing(0);

    m_copyrightLabel = new QLabel("© 2024 KINETIC. All rights reserved.", footer);
    m_copyrightLabel->setObjectName("copyrightLabel");

    m_termsButton = new QPushButton("Terms", footer);
    m_termsButton->setObjectName("termsButton");
    m_termsButton->setProperty("class", "footerLink");
    m_termsButton->setCursor(Qt::PointingHandCursor);
    m_termsButton->setFocusPolicy(Qt::NoFocus);

    m_privacyButton = new QPushButton("Privacy", footer);
    m_privacyButton->setObjectName("privacyButton");
    m_privacyButton->setProperty("class", "footerLink");
    m_privacyButton->setCursor(Qt::PointingHandCursor);
    m_privacyButton->setFocusPolicy(Qt::NoFocus);

    footerLayout->addWidget(m_copyrightLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(m_termsButton);
    footerLayout->addWidget(m_privacyButton);

    // ---------- Assemble main layout ----------
    mainLayout->addWidget(header);
    mainLayout->addStretch();

    // Center wrapper for card
    QHBoxLayout *cardWrapper = new QHBoxLayout();
    cardWrapper->addStretch();
    cardWrapper->addWidget(m_loginCard);
    cardWrapper->addStretch();
    mainLayout->addLayout(cardWrapper);

    mainLayout->addStretch();
    mainLayout->addWidget(footer);

    // Set internal spacing inside card
    cardLayout->setSpacing(4);

    // Add spacing between sections
    // Shield + titles
    QVBoxLayout *titleSection = new QVBoxLayout();
    titleSection->setSpacing(4);
    titleSection->addWidget(m_shieldIcon);
    titleSection->addWidget(m_titleLabel);
    titleSection->addWidget(m_subtitleLabel);
    titleSection->setAlignment(m_shieldIcon, Qt::AlignCenter);
    titleSection->setAlignment(m_titleLabel, Qt::AlignCenter);
    titleSection->setAlignment(m_subtitleLabel, Qt::AlignCenter);
    cardLayout->addLayout(titleSection);

    // Add some space before form
    cardLayout->addSpacing(24);

    // Username section
    QVBoxLayout *usernameSection = new QVBoxLayout();
    usernameSection->setSpacing(6);
    usernameSection->addWidget(m_usernameFieldLabel);
    usernameSection->addWidget(m_usernameGroup);
    cardLayout->addLayout(usernameSection);

    cardLayout->addSpacing(16);

    // Password section
    QVBoxLayout *passwordSection = new QVBoxLayout();
    passwordSection->setSpacing(6);
    passwordSection->addWidget(m_passwordFieldLabel);
    passwordSection->addWidget(m_passwordGroup);
    cardLayout->addLayout(passwordSection);

    cardLayout->addSpacing(12);

    // Forgot + Submit
    QVBoxLayout *actionSection = new QVBoxLayout();
    actionSection->setSpacing(12);
    actionSection->addWidget(m_forgotButton);
    actionSection->addWidget(m_submitButton);
    actionSection->setAlignment(m_forgotButton, Qt::AlignRight);
    cardLayout->addLayout(actionSection);

    // Divider + Sign up
    QVBoxLayout *bottomSection = new QVBoxLayout();
    bottomSection->setSpacing(8);
    bottomSection->addWidget(divider);
    bottomSection->addWidget(m_createAccountLabel);
    bottomSection->setAlignment(divider, Qt::AlignCenter);
    bottomSection->setAlignment(m_createAccountLabel, Qt::AlignCenter);
    cardLayout->addLayout(bottomSection);
}

void LoginWidget::setupConnections()
{
    connect(m_submitButton, &QPushButton::clicked, this, &LoginWidget::onSubmitClicked);
    connect(m_eyeButton, &QPushButton::clicked, this, &LoginWidget::onEyeClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &LoginWidget::onCloseClicked);
    connect(m_forgotButton, &QPushButton::clicked, this, &LoginWidget::forgotPasswordClicked);
    connect(m_createAccountLabel, &QLabel::linkActivated, this, &LoginWidget::signUpClicked);
    connect(m_termsButton, &QPushButton::clicked, this, []() {
        qDebug() << "Terms clicked";
    });
    connect(m_privacyButton, &QPushButton::clicked, this, []() {
        qDebug() << "Privacy clicked";
    });
}

void LoginWidget::onSubmitClicked()
{
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();
    emit loginSuccess(username, password);
}

void LoginWidget::onEyeClicked()
{
    m_passwordVisible = !m_passwordVisible;
    m_passwordEdit->setEchoMode(m_passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
    m_eyeButton->setText(m_passwordVisible ? "🙈" : "👁");
}

void LoginWidget::onCloseClicked()
{
    emit closeClicked();
}
