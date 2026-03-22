#include "TrayIcon.h"
#include <QDebug>
#include <QPixmap>
#include <QPainter>

TrayIcon::TrayIcon(QObject *parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_menu(nullptr)
    , m_currentStatus(ETrayStatus::Stopped)
    , m_startProxyAction(nullptr)
    , m_stopProxyAction(nullptr)
    , m_enableSystemProxyAction(nullptr)
    , m_disableSystemProxyAction(nullptr)
    , m_showWindowAction(nullptr)
    , m_settingsAction(nullptr)
    , m_exitAction(nullptr)
    , m_isSystemProxyEnabled(false)
{
}

TrayIcon::~TrayIcon()
{
    if (m_trayIcon)
    {
        m_trayIcon->hide();
        delete m_trayIcon;
        m_trayIcon = nullptr;
    }
    
    if (m_menu)
    {
        delete m_menu;
        m_menu = nullptr;
    }
}

void TrayIcon::init()
{
    // Create system tray icon
    m_trayIcon = new QSystemTrayIcon(this);
    
    // Set initial icon
    m_trayIcon->setIcon(createTrayIcon(m_currentStatus));
    m_trayIcon->setToolTip(QStringLiteral("XProxy"));
    
    // Create context menu
    m_menu = new QMenu();
    
    // Create menu actions
    m_startProxyAction = new QAction(QStringLiteral("启动代理"), this);
    m_stopProxyAction = new QAction(QStringLiteral("停止代理"), this);
    m_enableSystemProxyAction = new QAction(QStringLiteral("启用系统代理"), this);
    m_disableSystemProxyAction = new QAction(QStringLiteral("禁用系统代理"), this);
    m_showWindowAction = new QAction(QStringLiteral("显示主窗口"), this);
    m_settingsAction = new QAction(QStringLiteral("设置"), this);
    m_exitAction = new QAction(QStringLiteral("退出"), this);
    
    // Add actions to menu
    m_menu->addAction(m_startProxyAction);
    m_menu->addAction(m_stopProxyAction);
    m_menu->addSeparator();
    m_menu->addAction(m_enableSystemProxyAction);
    m_menu->addAction(m_disableSystemProxyAction);
    m_menu->addSeparator();
    m_menu->addAction(m_showWindowAction);
    m_menu->addAction(m_settingsAction);
    m_menu->addSeparator();
    m_menu->addAction(m_exitAction);
    
    // Set context menu
    m_trayIcon->setContextMenu(m_menu);
    
    // Connect signals
    connect(m_trayIcon, &QSystemTrayIcon::activated, 
            this, &TrayIcon::onTrayIconActivated);
    connect(m_startProxyAction, &QAction::triggered, 
            this, &TrayIcon::onStartProxy);
    connect(m_stopProxyAction, &QAction::triggered, 
            this, &TrayIcon::onStopProxy);
    connect(m_enableSystemProxyAction, &QAction::triggered, 
            this, &TrayIcon::onEnableSystemProxy);
    connect(m_disableSystemProxyAction, &QAction::triggered, 
            this, &TrayIcon::onDisableSystemProxy);
    connect(m_showWindowAction, &QAction::triggered, 
            this, &TrayIcon::onShowWindow);
    connect(m_settingsAction, &QAction::triggered, 
            this, &TrayIcon::onSettings);
    connect(m_exitAction, &QAction::triggered, 
            this, &TrayIcon::onExit);
    
    // Update menu state
    updateMenuState();
}

void TrayIcon::setStatus(ETrayStatus status)
{
    m_currentStatus = status;
    
    if (m_trayIcon)
    {
        m_trayIcon->setIcon(createTrayIcon(status));
    }
    
    updateMenuState();
}

void TrayIcon::show()
{
    if (m_trayIcon)
    {
        m_trayIcon->show();
    }
}

void TrayIcon::hide()
{
    if (m_trayIcon)
    {
        m_trayIcon->hide();
    }
}

void TrayIcon::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::Trigger:
            emit trayIconDoubleClicked();
            break;
        default:
            break;
    }
}

void TrayIcon::onStartProxy()
{
    emit startProxyClicked();
}

void TrayIcon::onStopProxy()
{
    emit stopProxyClicked();
}

void TrayIcon::onEnableSystemProxy()
{
    m_isSystemProxyEnabled = true;
    updateMenuState();
    emit enableSystemProxyClicked();
}

void TrayIcon::onDisableSystemProxy()
{
    m_isSystemProxyEnabled = false;
    updateMenuState();
    emit disableSystemProxyClicked();
}

void TrayIcon::onShowWindow()
{
    emit showWindowClicked();
}

void TrayIcon::onSettings()
{
    emit settingsClicked();
}

void TrayIcon::onExit()
{
    emit exitClicked();
}

QIcon TrayIcon::createTrayIcon(ETrayStatus status)
{
    // Create a 16x16 pixmap for the tray icon
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Set color based on status
    QColor color;
    switch (status)
    {
        case ETrayStatus::Running:
            color = QColor(0, 200, 0);    // Green
            break;
        case ETrayStatus::Disconnected:
            color = QColor(200, 0, 0);    // Red
            break;
        case ETrayStatus::Stopped:
        default:
            color = QColor(128, 128, 128); // Gray
            break;
    }
    
    // Draw circle
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(1, 1, 14, 14);
    
    // Draw inner circle (smaller)
    painter.setBrush(QBrush(Qt::white));
    painter.drawEllipse(4, 4, 8, 8);
    
    painter.end();
    
    return QIcon(pixmap);
}

void TrayIcon::updateMenuState()
{
    if (!m_menu)
        return;
    
    // Update proxy start/stop based on current status
    switch (m_currentStatus)
    {
        case ETrayStatus::Running:
            if (m_startProxyAction) m_startProxyAction->setEnabled(false);
            if (m_stopProxyAction) m_stopProxyAction->setEnabled(true);
            break;
        case ETrayStatus::Stopped:
        case ETrayStatus::Disconnected:
        default:
            if (m_startProxyAction) m_startProxyAction->setEnabled(true);
            if (m_stopProxyAction) m_stopProxyAction->setEnabled(false);
            break;
    }
    
    // Update system proxy state
    if (m_enableSystemProxyAction) 
        m_enableSystemProxyAction->setEnabled(!m_isSystemProxyEnabled);
    if (m_disableSystemProxyAction) 
        m_disableSystemProxyAction->setEnabled(m_isSystemProxyEnabled);
}
