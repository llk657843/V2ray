#include "LoginWidget.h"

#include <QFrame>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QSysInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "Crypt32.lib")
#endif

namespace {
QByteArray encryptBlob(const QByteArray &plain)
{
#ifdef Q_OS_WIN
    DATA_BLOB inputBlob;
    inputBlob.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(plain.constData()));
    inputBlob.cbData = static_cast<DWORD>(plain.size());

    DATA_BLOB outputBlob;
    if (!CryptProtectData(&inputBlob, L"v2raycpp_login_credential", nullptr, nullptr, nullptr, 0, &outputBlob)) {
        return QByteArray();
    }

    QByteArray encrypted(reinterpret_cast<const char *>(outputBlob.pbData), static_cast<int>(outputBlob.cbData));
    LocalFree(outputBlob.pbData);
    return encrypted;
#else
    const QByteArray key = QCryptographicHash::hash(QSysInfo::machineUniqueId(), QCryptographicHash::Sha256);
    if (key.isEmpty()) {
        return QByteArray();
    }

    QByteArray encrypted = plain;
    for (int i = 0; i < encrypted.size(); ++i) {
        encrypted[i] = encrypted[i] ^ key.at(i % key.size());
    }
    return encrypted.toBase64();
#endif
}

QByteArray decryptBlob(const QByteArray &cipher)
{
#ifdef Q_OS_WIN
    DATA_BLOB inputBlob;
    inputBlob.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(cipher.constData()));
    inputBlob.cbData = static_cast<DWORD>(cipher.size());

    DATA_BLOB outputBlob;
    if (!CryptUnprotectData(&inputBlob, nullptr, nullptr, nullptr, nullptr, 0, &outputBlob)) {
        return QByteArray();
    }

    QByteArray plain(reinterpret_cast<const char *>(outputBlob.pbData), static_cast<int>(outputBlob.cbData));
    LocalFree(outputBlob.pbData);
    return plain;
#else
    const QByteArray raw = QByteArray::fromBase64(cipher);
    const QByteArray key = QCryptographicHash::hash(QSysInfo::machineUniqueId(), QCryptographicHash::Sha256);
    if (key.isEmpty() || raw.isEmpty()) {
        return QByteArray();
    }

    QByteArray plain = raw;
    for (int i = 0; i < plain.size(); ++i) {
        plain[i] = plain[i] ^ key.at(i % key.size());
    }
    return plain;
#endif
}
} // namespace

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , m_passwordVisible(false)
    , m_pendingRemember(false)
{
    setupUi();
    setupConnections();
    loadRememberedCredentials();
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
    m_loginCard->setFixedSize(440, 520);

    QVBoxLayout *cardLayout = new QVBoxLayout(m_loginCard);
    cardLayout->setContentsMargins(34, 30, 34, 24);
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

    m_titleLabel = new QLabel(QStringLiteral("欢迎回来"), m_loginCard);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel(QStringLiteral("访问你的专属代理网络。"), m_loginCard);
    m_subtitleLabel->setObjectName("subtitleLabel");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    m_usernameFieldLabel = new QLabel(QStringLiteral("账号标识"), m_loginCard);
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
    m_usernameEdit->setPlaceholderText(QStringLiteral("请输入邮箱"));
    m_usernameEdit->setFrame(false);
    usernameLayout->addWidget(m_usernameEdit);

    QHBoxLayout *passwordHeaderLayout = new QHBoxLayout();
    passwordHeaderLayout->setContentsMargins(0, 0, 0, 0);
    passwordHeaderLayout->setSpacing(0);

    m_passwordFieldLabel = new QLabel(QStringLiteral("安全密钥"), m_loginCard);
    m_passwordFieldLabel->setObjectName("passwordFieldLabel");
    passwordHeaderLayout->addWidget(m_passwordFieldLabel);
    passwordHeaderLayout->addStretch();

    m_forgotButton = new QPushButton(QStringLiteral("忘记密码？"), m_loginCard);
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
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setFrame(false);
    passwordLayout->addWidget(m_passwordEdit);

    m_eyeButton = new QPushButton("o", m_passwordGroup);
    m_eyeButton->setObjectName("eyeButton");
    m_eyeButton->setFixedSize(22, 22);
    m_eyeButton->setCursor(Qt::PointingHandCursor);
    m_eyeButton->setFocusPolicy(Qt::NoFocus);
    passwordLayout->addWidget(m_eyeButton);

    m_rememberCheckBox = new QCheckBox(QStringLiteral("记住密码"), m_loginCard);
    m_rememberCheckBox->setObjectName("rememberPasswordCheckBox");
    m_rememberCheckBox->setCursor(Qt::PointingHandCursor);
    m_rememberCheckBox->setFocusPolicy(Qt::NoFocus);

    m_submitButton = new QPushButton(QStringLiteral("登录"), m_loginCard);
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
    m_createAccountHint = new QLabel(QStringLiteral("还没有账号？ "), m_loginCard);
    m_createAccountHint->setObjectName("loginCreateAccountHint");
    m_createAccountButton = new QPushButton(QStringLiteral("注册账号"), m_loginCard);
    m_createAccountButton->setObjectName("loginCreateAccountLink");
    m_createAccountButton->setFlat(true);
    m_createAccountButton->setCursor(Qt::PointingHandCursor);
    m_createAccountButton->setFocusPolicy(Qt::NoFocus);
    createAccountRow->addStretch();
    createAccountRow->addWidget(m_createAccountHint);
    createAccountRow->addWidget(m_createAccountButton);
    createAccountRow->addStretch();

    cardLayout->addSpacing(4);
    cardLayout->addWidget(shieldContainer, 0, Qt::AlignCenter);
    cardLayout->addSpacing(12);
    cardLayout->addWidget(m_titleLabel);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(m_subtitleLabel);
    cardLayout->addSpacing(22);
    cardLayout->addWidget(m_usernameFieldLabel);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(m_usernameGroup);
    cardLayout->addSpacing(12);
    cardLayout->addLayout(passwordHeaderLayout);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(m_passwordGroup);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(m_rememberCheckBox);
    cardLayout->addSpacing(18);
    cardLayout->addWidget(m_submitButton);
    cardLayout->addSpacing(20);
    cardLayout->addWidget(separator);
    cardLayout->addSpacing(16);
    cardLayout->addLayout(createAccountRow);
    cardLayout->addStretch();

    mainLayout->addWidget(m_loginCard, 0, Qt::AlignCenter);
}

void LoginWidget::setupConnections()
{
    connect(m_submitButton, &QPushButton::clicked, this, &LoginWidget::onSubmitClicked);
    connect(m_eyeButton, &QPushButton::clicked, this, &LoginWidget::onEyeClicked);
    connect(m_forgotButton, &QPushButton::clicked, this, &LoginWidget::forgotPasswordClicked);
    connect(m_rememberCheckBox, &QCheckBox::toggled, this, &LoginWidget::onRememberToggled);
    connect(m_createAccountButton, &QPushButton::clicked, this, [this]() {
        emit signUpClicked();
    });
}

void LoginWidget::onSubmitClicked()
{
    stageCredentialsForLoginAttempt();
    emit loginSuccess(m_usernameEdit->text(), m_passwordEdit->text());
}

void LoginWidget::onEyeClicked()
{
    m_passwordVisible = !m_passwordVisible;
    m_passwordEdit->setEchoMode(m_passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
    m_eyeButton->setText(m_passwordVisible ? "-" : "o");
}

void LoginWidget::onRememberToggled(bool checked)
{
    if (!checked) {
        clearCredentialFile();
    }
}

void LoginWidget::loadRememberedCredentials()
{
    QString rememberedUsername;
    QString rememberedPassword;
    if (!readEncryptedCredentials(rememberedUsername, rememberedPassword)) {
        m_rememberCheckBox->setChecked(false);
        return;
    }

    m_rememberCheckBox->setChecked(true);
    m_usernameEdit->setText(rememberedUsername);
    m_passwordEdit->setText(rememberedPassword);
}

void LoginWidget::stageCredentialsForLoginAttempt()
{
    m_pendingRemember = m_rememberCheckBox->isChecked();
    m_pendingUsername = m_usernameEdit->text();
    m_pendingPassword = m_passwordEdit->text();
}

void LoginWidget::persistStagedCredentialsIfNeeded()
{
    if (!m_pendingRemember) {
        clearCredentialFile();
        return;
    }

    if (!writeEncryptedCredentials(m_pendingUsername, m_pendingPassword)) {
        clearCredentialFile();
    }
}

QString LoginWidget::credentialFilePath() const
{
    const QString appDataPath = qEnvironmentVariable("APPDATA");
    if (!appDataPath.isEmpty()) {
        return QDir(appDataPath).filePath(QStringLiteral("v2raycpp/login_credentials.dat"));
    }
    const QString fallbackPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(fallbackPath).filePath(QStringLiteral("login_credentials.dat"));
}

bool LoginWidget::writeEncryptedCredentials(const QString &username, const QString &password)
{
    const QString filePath = credentialFilePath();
    const QFileInfo fileInfo(filePath);
    QDir dir(fileInfo.absolutePath());
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("username"), username);
    payload.insert(QStringLiteral("password"), password);

    const QByteArray plain = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    const QByteArray encrypted = encryptBlob(plain);
    if (encrypted.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    const qint64 written = file.write(encrypted);
    file.close();
    return written == encrypted.size();
}

bool LoginWidget::readEncryptedCredentials(QString &username, QString &password)
{
    QFile file(credentialFilePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray encrypted = file.readAll();
    file.close();
    if (encrypted.isEmpty()) {
        return false;
    }

    const QByteArray plain = decryptBlob(encrypted);
    if (plain.isEmpty()) {
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(plain, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    const QJsonObject obj = doc.object();
    username = obj.value(QStringLiteral("username")).toString();
    password = obj.value(QStringLiteral("password")).toString();
    return !username.isEmpty();
}

void LoginWidget::clearCredentialFile()
{
    QFile::remove(credentialFilePath());
}

void LoginWidget::onLoginResult(bool success)
{
    if (success) {
        persistStagedCredentialsIfNeeded();
    }

    m_pendingRemember = false;
    m_pendingUsername.clear();
    m_pendingPassword.clear();
}
