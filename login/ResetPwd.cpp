#include "ResetPwd.h"
#include "ui_resetPwd.h"

#include <QLineEdit>

ResetPwd::ResetPwd(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ResetPwd)
{
    ui->setupUi(this);
    connect(ui->sendButton, &QPushButton::clicked, this, &ResetPwd::onSendClicked);
    connect(ui->backToLoginButton, &QPushButton::clicked, this, &ResetPwd::backToLoginRequested);
}

ResetPwd::~ResetPwd()
{
    delete ui;
}

QString ResetPwd::accountIdentifier() const
{
    return ui->accountInput->text().trimmed();
}

void ResetPwd::clearAccountIdentifier()
{
    ui->accountInput->clear();
}

void ResetPwd::onSendClicked()
{
    const QString id = accountIdentifier();
    if (!id.isEmpty()) {
        emit sendResetLinkRequested(id);
    }
}
