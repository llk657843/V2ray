#ifndef SERVERCARDWIDGET_H
#define SERVERCARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QStyle>

class ServerCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ServerCardWidget(QWidget *parent = nullptr);
    ~ServerCardWidget();

    void setNodeInfo(const QString &name, int latency, const QString &protocol, bool connected);
    void setConnected(bool connected);
    void setFlag(const QString &flagPath);

signals:
    void clicked();
    void toggled(bool checked);

private slots:
    void onConnectToggled(bool checked);

private:
    void setupUi();
    bool eventFilter(QObject *obj, QEvent *event) override;
    void updateConnectionState(bool connected);

    QLabel *m_flagLabel;
    QLabel *m_nameLabel;
    QLabel *m_latencyLabel;
    QLabel *m_protocolLabel;
    QPushButton *m_connectBtn;
};

#endif // SERVERCARDWIDGET_H
