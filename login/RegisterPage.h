#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class RegisterPage;
}
QT_END_NAMESPACE

class RegisterPage : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterPage(QWidget *parent = nullptr);
    ~RegisterPage() override;

    QString email() const;
    QString password() const;
    QString verifyPassword() const;

signals:
    void accountInitializeRequested(const QString &email, const QString &password);
    void backToLoginRequested();

private slots:
    void onInitializeClicked();
    void onPasswordEyeClicked();

private:
    Ui::RegisterPage *ui;
    bool m_passwordVisible;
};

#endif // REGISTERPAGE_H
