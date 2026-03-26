#ifndef VERIFYCODEPAGE_H
#define VERIFYCODEPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class VerifyCodePage;
}
QT_END_NAMESPACE

class QLineEdit;

class VerifyCodePage : public QWidget
{
    Q_OBJECT

public:
    explicit VerifyCodePage(QWidget *parent = nullptr);
    ~VerifyCodePage() override;

    QString code() const;
    QString newPassword() const;
    void clearCode();
    void clearPasswordFields();

signals:
    void verifyRequested(const QString &code, const QString &newPassword);
    void resendRequested();

private slots:
    void onVerifyClicked();
    void onResendLinkActivated(const QString &link);

private:
    void wireDigitInputs();
    void advanceIfDigit(int index, const QString &text);
    bool eventFilter(QObject *watched, QEvent *event) override;

    Ui::VerifyCodePage *ui;
    QLineEdit *m_digits[6];
};

#endif // VERIFYCODEPAGE_H
