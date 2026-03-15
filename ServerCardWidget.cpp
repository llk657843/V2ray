#include "ServerCardWidget.h"
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QFile>
#include <QDebug>

ServerCardWidget::ServerCardWidget(QWidget *parent)
    : QWidget(parent)
    , m_flagLabel(new QLabel(this))
    , m_nameLabel(new QLabel(this))
    , m_latencyLabel(new QLabel(this))
    , m_protocolLabel(new QLabel(this))
    , m_connectBtn(new QPushButton(this))
{
    setupUi();
    installEventFilter(this);
}

ServerCardWidget::~ServerCardWidget()
{
}

void ServerCardWidget::setupUi()
{
    setFixedHeight(72);
    setContentsMargins(12, 8, 12, 8);

    // Card style
    setStyleSheet(R"(
        ServerCardWidget {
            background-color: #f5f5f5;
            border: 1px solid #e0e0e0;
            border-radius: 12px;
        }
        ServerCardWidget:hover {
            background-color: #eeeeee;
        }
    )");

    // Flag label
    m_flagLabel->setObjectName("flagLabel");
    m_flagLabel->setFixedSize(32, 24);
    m_flagLabel->setScaledContents(true);

    // Name label
    m_nameLabel->setObjectName("nameLabel");
    m_nameLabel->setText("Server");
    m_nameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");

    // Latency label
    m_latencyLabel->setObjectName("latencyLabel");
    m_latencyLabel->setText("-- ms");
    m_latencyLabel->setStyleSheet("font-size: 12px; color: #666666;");

    // Protocol label
    m_protocolLabel->setObjectName("protocolLabel");
    m_protocolLabel->setText("VMess");
    m_protocolLabel->setStyleSheet("font-size: 11px; color: #999999; background-color: #e0e0e0; padding: 2px 6px; border-radius: 4px;");

    // Connect button - toggle switch style
    m_connectBtn->setFixedSize(44, 22);
    m_connectBtn->setCheckable(true);
    m_connectBtn->setCursor(Qt::PointingHandCursor);
    m_connectBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #cccccc;
            border: none;
            border-radius: 11px;
        }
        QPushButton:checked {
            background-color: #1152d4;
        }
    )");
    m_connectBtn->setText("");

    connect(m_connectBtn, &QPushButton::toggled, this, &ServerCardWidget::onConnectToggled);
}

void ServerCardWidget::setNodeInfo(const QString &name, int latency, const QString &protocol, bool connected)
{
    m_nameLabel->setText(name);
    m_protocolLabel->setText(protocol);

    // Latency with color
    QString latencyColor;
    if (latency < 0) {
        m_latencyLabel->setText("-- ms");
        latencyColor = "#666666";
    } else if (latency < 100) {
        m_latencyLabel->setText(QString::number(latency) + " ms");
        latencyColor = "#4caf50"; // Green
    } else if (latency < 200) {
        m_latencyLabel->setText(QString::number(latency) + " ms");
        latencyColor = "#ff9800"; // Orange/Yellow
    } else {
        m_latencyLabel->setText(QString::number(latency) + " ms");
        latencyColor = "#f44336"; // Red
    }
    m_latencyLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(latencyColor));

    setConnected(connected);
}

void ServerCardWidget::setConnected(bool connected)
{
    m_connectBtn->setChecked(connected);
    updateConnectionState(connected);
}

void ServerCardWidget::setFlag(const QString &flagPath)
{
    if (flagPath.isEmpty()) {
        m_flagLabel->setText("🌐");
        return;
    }

    QPixmap pixmap(flagPath);
    if (!pixmap.isNull()) {
        m_flagLabel->setPixmap(pixmap);
    } else {
        m_flagLabel->setText("🌐");
    }
}

void ServerCardWidget::onConnectToggled(bool checked)
{
    updateConnectionState(checked);
    emit toggled(checked);
}

bool ServerCardWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this) {
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                if (event->type() == QEvent::MouseButtonPress) {
                    // Check if click is on the toggle button
                    QRect btnRect = m_connectBtn->geometry();
                    if (!btnRect.contains(mouseEvent->pos())) {
                        emit clicked();
                    }
                }
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ServerCardWidget::updateConnectionState(bool connected)
{
    if (connected) {
        // Connected: left border 3px main color #1152d4, darker background
        setStyleSheet(R"(
            ServerCardWidget {
                background-color: #e8f0fe;
                border: 1px solid #1152d4;
                border-left: 3px solid #1152d4;
                border-radius: 12px;
            }
            ServerCardWidget:hover {
                background-color: #d4e4fc;
            }
        )");
    } else {
        // Not connected: gray border, light background
        setStyleSheet(R"(
            ServerCardWidget {
                background-color: #f5f5f5;
                border: 1px solid #e0e0e0;
                border-radius: 12px;
            }
            ServerCardWidget:hover {
                background-color: #eeeeee;
            }
        )");
    }
}
