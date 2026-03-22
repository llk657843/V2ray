#ifndef RESETPWD_H
#define RESETPWD_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class ResetPwd;
}
QT_END_NAMESPACE

class ResetPwd : public QWidget
{
    Q_OBJECT

public:
    explicit ResetPwd(QWidget *parent = nullptr);
    ~ResetPwd() override;

    QString accountIdentifier() const;
    void clearAccountIdentifier();

signals:
    void sendResetLinkRequested(const QString &identifier);
    void backToLoginRequested();

private slots:
    void onSendClicked();

private:
    Ui::ResetPwd *ui;
};

#endif // RESETPWD_H
