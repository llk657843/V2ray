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

private slots:
    void onCloseClicked();

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
