#ifndef LoginMainWidget_H
#define LoginMainWidget_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QMouseEvent>
#include <QMap>
#include <QStackedWidget>

class LoginWidget;

class LoginMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginMainWidget(QWidget *parent = nullptr);
    ~LoginMainWidget();

    void addPage(const QString &name, QWidget *widget);
    void switchPage(const QString &name);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void loginSuccess();
    void loginClose();
    void verifyCodeSubmitted(const QString &code);
    void resendVerificationRequested();
    void resetLinkRequested(const QString &identifier);
    void accountInitializeRequested(const QString &email, const QString &password);

private slots:
    void onCloseClicked();
    void onVerifyCodeSubmitted(const QString &code);
    void onAccountInitializeRequested(const QString &email, const QString &password);

private:
    void setupUi();

    // Content
    QStackedWidget *m_stackedWidget;

    // Page management
    QMap<QString, QWidget *> m_pages;

    // Window dragging
    bool m_dragging;
    QPoint m_dragPosition;
};

#endif // LoginMainWidget_H
