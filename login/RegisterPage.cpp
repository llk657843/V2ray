#include "RegisterPage.h"
#include "ui_registerPage.h"

RegisterPage::RegisterPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterPage)
    , m_passwordVisible(false)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    ui->registerCard->setAttribute(Qt::WA_StyledBackground, true);
    ui->emailInputGroup->setAttribute(Qt::WA_StyledBackground, true);
    ui->passwordInputGroup->setAttribute(Qt::WA_StyledBackground, true);
    ui->verifyPasswordInputGroup->setAttribute(Qt::WA_StyledBackground, true);

    connect(ui->initializeAccountBtn, &QPushButton::clicked, this, &RegisterPage::onInitializeClicked);
    connect(ui->backToLoginBtn, &QPushButton::clicked, this, &RegisterPage::backToLoginRequested);
    connect(ui->passwordEyeButton, &QPushButton::clicked, this, &RegisterPage::onPasswordEyeClicked);
}

RegisterPage::~RegisterPage()
{
    delete ui;
}

QString RegisterPage::email() const
{
    return ui->emailLineEdit->text().trimmed();
}

QString RegisterPage::password() const
{
    return ui->passwordLineEdit->text();
}

QString RegisterPage::verifyPassword() const
{
    return ui->verifyPasswordLineEdit->text();
}

void RegisterPage::onInitializeClicked()
{
    const QString e = email();
    const QString p = password();
    const QString v = verifyPassword();
    if (e.isEmpty() || p.isEmpty() || v.isEmpty()) {
        return;
    }
    if (p != v) {
        return;
    }
    emit accountInitializeRequested(e, p);
}

void RegisterPage::onPasswordEyeClicked()
{
    m_passwordVisible = !m_passwordVisible;
    ui->passwordLineEdit->setEchoMode(m_passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
    ui->passwordEyeButton->setText(m_passwordVisible ? QStringLiteral("-") : QStringLiteral("o"));
}
