#include "VerifyCodePage.h"
#include "ui_VerifyCodePage.h"

#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpressionValidator>

VerifyCodePage::VerifyCodePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VerifyCodePage)
{
    ui->setupUi(this);

    m_digits[0] = ui->digit0;
    m_digits[1] = ui->digit1;
    m_digits[2] = ui->digit2;
    m_digits[3] = ui->digit3;
    m_digits[4] = ui->digit4;
    m_digits[5] = ui->digit5;

    const QRegularExpression rx(QStringLiteral("[0-9]"));
    for (int i = 0; i < 6; ++i) {
        m_digits[i]->setFrame(false);
        m_digits[i]->installEventFilter(this);
        m_digits[i]->setValidator(new QRegularExpressionValidator(rx, m_digits[i]));
    }

    ui->resendLabel->setCursor(Qt::PointingHandCursor);

    connect(ui->verifyButton, &QPushButton::clicked, this, &VerifyCodePage::onVerifyClicked);
    connect(ui->resendLabel, &QLabel::linkActivated, this, &VerifyCodePage::onResendLinkActivated);

    wireDigitInputs();
}

VerifyCodePage::~VerifyCodePage()
{
    delete ui;
}

QString VerifyCodePage::code() const
{
    QString s;
    s.reserve(6);
    for (int i = 0; i < 6; ++i) {
        s += m_digits[i]->text();
    }
    return s;
}

QString VerifyCodePage::newPassword() const
{
    return ui->newPasswordInput->text();
}

void VerifyCodePage::clearCode()
{
    for (int i = 0; i < 6; ++i) {
        m_digits[i]->clear();
    }
    m_digits[0]->setFocus();
}

void VerifyCodePage::clearPasswordFields()
{
    ui->newPasswordInput->clear();
    ui->confirmPasswordInput->clear();
}

void VerifyCodePage::wireDigitInputs()
{
    for (int i = 0; i < 6; ++i) {
        connect(m_digits[i], &QLineEdit::textChanged, this, [this, i](const QString &text) {
            advanceIfDigit(i, text);
        });
    }
}

void VerifyCodePage::advanceIfDigit(int index, const QString &text)
{
    if (text.length() > 1) {
        const QString one = text.left(1);
        m_digits[index]->blockSignals(true);
        m_digits[index]->setText(one);
        m_digits[index]->blockSignals(false);
    }
    if (!text.isEmpty() && index < 5) {
        m_digits[index + 1]->setFocus();
    }
}

void VerifyCodePage::onVerifyClicked()
{
    const QString c = code();
    const QString password = ui->newPasswordInput->text();
    const QString confirmPassword = ui->confirmPasswordInput->text();

    if (c.length() != 6) {
        QMessageBox::warning(this, tr("验证码错误"), tr("请输入 6 位验证码。"));
        return;
    }

    if (password.length() < 8) {
        QMessageBox::warning(this, tr("密码不合规"), tr("新密码长度不能少于 8 位。"));
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, tr("密码不一致"), tr("两次输入的新密码不一致，请重新输入。"));
        return;
    }

    emit verifyRequested(c, password);
}

void VerifyCodePage::onResendLinkActivated(const QString &link)
{
    if (link == QLatin1String("resend")) {
        emit resendRequested();
    }
}

bool VerifyCodePage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(event);
        for (int i = 0; i < 6; ++i) {
            if (watched != m_digits[i]) {
                continue;
            }
            if (ke->key() == Qt::Key_Backspace && m_digits[i]->text().isEmpty() && i > 0) {
                m_digits[i - 1]->clear();
                m_digits[i - 1]->setFocus();
                return true;
            }
            if (ke->key() == Qt::Key_Left && i > 0) {
                m_digits[i - 1]->setFocus();
                return true;
            }
            if (ke->key() == Qt::Key_Right && i < 5) {
                m_digits[i + 1]->setFocus();
                return true;
            }
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}
