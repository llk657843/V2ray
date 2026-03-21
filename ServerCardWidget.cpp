#include "ServerCardWidget.h"
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QStyle>

namespace {
void polishWidget(QWidget *w)
{
    if (!w || !w->style())
        return;
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}
} // namespace

ServerCardWidget::ServerCardWidget(QWidget *parent)
    : QWidget(parent)
    , m_flagLabel(new QLabel(this))
    , m_nameLabel(new QLabel(this))
    , m_latencyLabel(new QLabel(this))
    , m_protocolLabel(new QLabel(this))
    , m_connectBtn(new QPushButton(this))
{
    setAttribute(Qt::WA_StyledBackground, true);
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

    setProperty("connected", false);

    // Flag label
    m_flagLabel->setObjectName("serverCardFlagLabel");
    m_flagLabel->setFixedSize(32, 24);
    m_flagLabel->setScaledContents(true);

    // Name label
    m_nameLabel->setObjectName("serverCardNameLabel");
    m_nameLabel->setText("Server");

    // Latency label
    m_latencyLabel->setObjectName("serverCardLatencyLabel");
    m_latencyLabel->setText("-- ms");
    m_latencyLabel->setProperty("latencyTier", "unknown");

    // Protocol label
    m_protocolLabel->setObjectName("serverCardProtocolLabel");
    m_protocolLabel->setText("VMess");

    // Connect button - toggle switch style (styled in serverCard.qss)
    m_connectBtn->setObjectName("serverCardConnectBtn");
    m_connectBtn->setFixedSize(44, 22);
    m_connectBtn->setCheckable(true);
    m_connectBtn->setCursor(Qt::PointingHandCursor);
    m_connectBtn->setText("");

    connect(m_connectBtn, &QPushButton::toggled, this, &ServerCardWidget::onConnectToggled);

    polishWidget(this);
    polishWidget(m_latencyLabel);
}

void ServerCardWidget::setNodeInfo(const QString &name, int latency, const QString &protocol, bool connected)
{
    m_nameLabel->setText(name);
    m_protocolLabel->setText(protocol);

    QString tier = QStringLiteral("unknown");
    if (latency < 0) {
        m_latencyLabel->setText("-- ms");
        tier = QStringLiteral("unknown");
    } else if (latency < 100) {
        m_latencyLabel->setText(QString::number(latency) + " ms");
        tier = QStringLiteral("good");
    } else if (latency < 200) {
        m_latencyLabel->setText(QString::number(latency) + " ms");
        tier = QStringLiteral("mid");
    } else {
        m_latencyLabel->setText(QString::number(latency) + " ms");
        tier = QStringLiteral("bad");
    }
    m_latencyLabel->setProperty("latencyTier", tier);
    polishWidget(m_latencyLabel);

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
    setProperty("connected", connected);
    polishWidget(this);
}
