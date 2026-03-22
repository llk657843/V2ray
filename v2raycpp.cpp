#include "v2raycpp.h"
#include <QMouseEvent>
#include <QIcon>
#include <QSize>
#include <QPixmap>
#include <QPainter>
#include "TrayIcon.h"
#include "ConfigGenerator.h"
#include "XrayConfigStore.h"
#include "ProfileManager.h"
#include <QFileDialog>
#include <QDateTime>
#include <QTextEdit>
#include <QPushButton>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QClipboard>
#include <QInputDialog>
#include <QShortcut>
#include <Qt>
#include <QPointer>
#include "ServerCardProbeService.h"
#include "TrafficStatsController.h"
#include "ReconnectController.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include "SimpleCard.h"
#include "FooterBarIcons.h"

namespace {
QString protocolSearchKeyword(EConfigType t)
{
    switch (t) {
    case EConfigType::VMess:
        return QStringLiteral("vmess");
    case EConfigType::VLESS:
        return QStringLiteral("vless");
    case EConfigType::Trojan:
        return QStringLiteral("trojan");
    case EConfigType::Shadowsocks:
        return QStringLiteral("shadowsocks");
    default:
        return QStringLiteral("other");
    }
}

QString formatFooterUptime(const QDateTime& started, CoreStatus status)
{
    if (status != CoreStatus::Running || !started.isValid()) {
        return QStringLiteral("UPTIME: --");
    }
    const qint64 secs = qMax(qint64(0), started.secsTo(QDateTime::currentDateTime()));
    const qint64 h = secs / 3600;
    const int m = static_cast<int>((secs % 3600) / 60);
    const int s = static_cast<int>(secs % 60);
    const QString hp = (h < 10) ? QStringLiteral("0%1").arg(h) : QString::number(h);
    return QStringLiteral("UPTIME: %1:%2:%3")
        .arg(hp)
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(s, 2, 10, QLatin1Char('0'));
}

constexpr int kLatencyProbePending = -2;
constexpr int kSidebarNavIconPx = 20;
constexpr int kSidebarNavIconTrailingPad = 8;

QIcon makeSidebarNavIcon(const QString& imagePath, int iconPx, int trailingGapPx)
{
    QPixmap pm(imagePath);
    if (pm.isNull())
        return QIcon(imagePath);
    const int w = iconPx + qMax(0, trailingGapPx);
    const int h = iconPx;
    QPixmap canvas(w, h);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    const QPixmap scaled = pm.scaled(iconPx, iconPx, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, scaled);
    return QIcon(canvas);
}

void applyLabelIcon(QLabel* label, const QString& qrcPath, int px)
{
    if (!label) {
        return;
    }
    QPixmap pm(qrcPath);
    if (pm.isNull()) {
        return;
    }
    label->setPixmap(pm.scaled(px, px, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label->setScaledContents(false);
}
} // namespace

v2raycpp::v2raycpp(QWidget *parent)
    : QWidget(parent)
{
    m_dragging = false;
    m_dragPosition = QPoint();
    
    setWindowFlags(Qt::FramelessWindowHint);
    
    // Load window position from config
    {
        QString configPath = QCoreApplication::applicationDirPath() + "/../../window_pos.json";
        QFile file(configPath);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                // 检查所有必要的 key 是否存在
                if (obj.contains("x") && obj.contains("y") && obj.contains("width") && obj.contains("height")) {
                    int x = obj["x"].toInt(-1);
                    int y = obj["y"].toInt(-1);
                    int w = obj["width"].toInt(-1);
                    int h = obj["height"].toInt(-1);
                    if (x >= 0 && y >= 0 && w > 0 && h > 0) {
                        setGeometry(x, y, w, h);
                    }
                }
            }
        }
    }
    
    ui.setupUi(this);
    setObjectName(QStringLiteral("ProxyClient"));

    // Set sidebar fixed width to 256px per Figma design
    if (ui.sidebarLayout && ui.sidebarLayout->parentWidget()) {
        ui.sidebarLayout->parentWidget()->setMinimumWidth(256);
    }
    
    // Initialize core manager
    m_coreManager = std::make_unique<CoreManager>();
    
    // Initialize system proxy handler
    m_sysProxyHandler = std::make_unique<SysProxyHandler>();
    
    // Initialize tray icon
    m_trayIcon = std::make_unique<TrayIcon>(this);
    m_trayIcon->init();
    m_trayIcon->show();

    m_serverProbe = new ServerCardProbeService(this);
    connect(m_serverProbe, &ServerCardProbeService::latencyMeasured, this, &v2raycpp::onProbeLatency);
    connect(m_serverProbe, &ServerCardProbeService::geoFinished, this, &v2raycpp::onProbeGeo);

    m_trafficStats = new TrafficStatsController(this);
    connect(m_trafficStats, &TrafficStatsController::speedsUpdated, this, [this](const QString& down, const QString& up) {
        if (ui.statSpeedDown) {
            ui.statSpeedDown->setText(down);
        }
        if (ui.statSpeedUp) {
            ui.statSpeedUp->setText(up);
        }
        if (ui.statUptime) {
            ui.statUptime->setText(formatFooterUptime(m_startTime, m_currentStatus));
        }
    });

    m_reconnectController = new ReconnectController(this);
    connect(m_reconnectController, &ReconnectController::reconnectDue, this, [this]() {
        qDebug() << "Auto reconnecting...";
        onStartClicked();
    });

    // Initialize UI (styles come from QApplication stylesheet in main.cpp)
    initUI();
    initServerGrid(); // 鍒濆鍖栧崱鐗囩綉鏍?
    
    // Initialize connections
    initConnections();
    
    // Load configuration
    loadConfig();
    
    // Update UI status
    updateUIStatus();
    
    // Connect tray icon signals
    connect(m_trayIcon.get(), &TrayIcon::startProxyClicked, this, &v2raycpp::onStartClicked);
    connect(m_trayIcon.get(), &TrayIcon::stopProxyClicked, this, &v2raycpp::onStopClicked);
    connect(m_trayIcon.get(), &TrayIcon::showWindowClicked, this, &v2raycpp::onShowWindowFromTray);
    connect(m_trayIcon.get(), &TrayIcon::settingsClicked, this, &v2raycpp::onSettingsFromTray);
    connect(m_trayIcon.get(), &TrayIcon::exitClicked, this, &v2raycpp::onExitFromTray);
    connect(m_trayIcon.get(), &TrayIcon::trayIconDoubleClicked, this, &v2raycpp::onTrayDoubleClicked);
    connect(m_trayIcon.get(), &TrayIcon::enableSystemProxyClicked, this, &v2raycpp::onEnableSystemProxy);
    connect(m_trayIcon.get(), &TrayIcon::disableSystemProxyClicked, this, &v2raycpp::onDisableSystemProxy);
}

v2raycpp::~v2raycpp()
{
    // Save configuration before exit
    saveConfig();
    
    // Stop core if running
    if (m_coreManager && m_coreManager->isRunning())
    {
        m_coreManager->stopCore();
    }
    
    // Clear system proxy
    if (m_sysProxyHandler)
    {
        m_sysProxyHandler->clearProxy();
    }
    
    // Hide tray icon
    if (m_trayIcon)
    {
        m_trayIcon->hide();
    }
}

void v2raycpp::mousePressEvent(QMouseEvent* event)
{
    // 拖拽顶部 40 像素区域移动窗口
    if (event->pos().y() < 40) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void v2raycpp::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void v2raycpp::mouseReleaseEvent(QMouseEvent* event)
{
    m_dragging = false;
}

void v2raycpp::closeEvent(QCloseEvent* event)
{
    // Save window position
    QString configPath = QCoreApplication::applicationDirPath() + "/../../window_pos.json";
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject obj;
        obj["x"] = x();
        obj["y"] = y();
        obj["width"] = width();
        obj["height"] = height();
        QJsonDocument doc(obj);
        file.write(doc.toJson());
    }
    
    // Stop reconnect timer
    stopReconnectTimer();
    
    // Stop stats timer
    stopStatsTimer();
    
    // Stop core if running
    if (m_currentStatus == CoreStatus::Running) {
        onStopClicked();
    }
    
    event->accept();
}

void v2raycpp::initServerGrid()
{
    // NOTE: Server grid is created via UI file (v2raycpp.ui)
    // This function is kept for future customization if needed
}

void v2raycpp::initUI()
{
    // Set window title
    setWindowTitle(QStringLiteral("XProxy"));
    
    // Load PNG icons from images/figma directory (relative to exe)
    QString iconPath = QCoreApplication::applicationDirPath() + "/../../images/figma/";

    // #region agent log
    {
        auto agentDebugLog = [](const QString& hypothesisId, const QString& location, const QString& message) {
            QFile f("debug-170cfb.log");
            if (!f.open(QIODevice::Append | QIODevice::Text)) {
                return;
            }
            QTextStream ts(&f);
            const qint64 tsNow = QDateTime::currentMSecsSinceEpoch();
            const QString json = QString("{\"sessionId\":\"170cfb\",\"runId\":\"pre-fix\",\"hypothesisId\":\"%1\",\"location\":\"%2\",\"message\":\"%3\",\"data\":{},\"timestamp\":%4}")
                                     .arg(hypothesisId, location, message)
                                     .arg(tsNow);
            ts << json << "\n";
        };

        const bool iconsDirExists = QDir(iconPath).exists();
        agentDebugLog("H1", "v2raycpp.cpp:215", QString("iconPathExists=%1").arg(iconsDirExists));

        QIcon testIcon = QIcon(iconPath + "nav_home.png");
        agentDebugLog("H2", "v2raycpp.cpp:222", QString("testIconIsNull=%1").arg(testIcon.isNull()));

        QPixmap testPixmap(iconPath + "status_connect.png");
        agentDebugLog("H3", "v2raycpp.cpp:278", QString("testPixmapIsNull=%1").arg(testPixmap.isNull()));
    }
    // #endregion agent log

    // Helper to load PNG icon (explicit capture)
    auto loadIcon = [iconPath](const QString& name) {
        return QIcon(iconPath + name + ".png");
    };
    auto loadNavIcon = [iconPath](const QString& name) -> QIcon {
        const QString diskPath = iconPath + name + QStringLiteral(".png");
        QIcon icon = makeSidebarNavIcon(diskPath, kSidebarNavIconPx, kSidebarNavIconTrailingPad);
        if (!icon.isNull())
            return icon;
        const QString qrcPath = QStringLiteral(":/images/figma/%1.png").arg(name);
        return makeSidebarNavIcon(qrcPath, kSidebarNavIconPx, kSidebarNavIconTrailingPad);
    };

    if (ui.searchIcon && ui.searchIcon->parentWidget()) {
        ui.searchIcon->parentWidget()->setAttribute(Qt::WA_StyledBackground, true);
    }
    constexpr int kSearchIconPx = 12;
    applyLabelIcon(ui.searchIcon, QStringLiteral(":/images/figma/icon_search.png"), kSearchIconPx);

    constexpr int kBarIconPx = 14;
    applyLabelIcon(ui.statConnIcon, FooterBarIcons::connection(), kBarIconPx);
    applyLabelIcon(ui.statSpeedUpIcon, FooterBarIcons::uploadSpeed(), kBarIconPx);
    applyLabelIcon(ui.statSpeedDownIcon, FooterBarIcons::downloadSpeed(), kBarIconPx);
    applyLabelIcon(ui.statIPIcon, FooterBarIcons::globeIp(), kBarIconPx);
    applyLabelIcon(ui.statUptimeIcon, FooterBarIcons::clockUptime(), kBarIconPx);
    
    // Sidebar navigation icons
    const QSize navIconSize(kSidebarNavIconPx + kSidebarNavIconTrailingPad, kSidebarNavIconPx);
    if (ui.navHome) {
        ui.navHome->setText(QStringLiteral("首页"));
        ui.navHome->setIcon(loadNavIcon(QStringLiteral("nav_home")));
        ui.navHome->setIconSize(navIconSize);
    }
    if (ui.navServers) {
        ui.navServers->setText(QStringLiteral("服务器"));
        ui.navServers->setIcon(loadNavIcon(QStringLiteral("nav_servers")));
        ui.navServers->setIconSize(navIconSize);
    }
    if (ui.navSettings) {
        ui.navSettings->setText(QStringLiteral("设置"));
        ui.navSettings->setIcon(loadNavIcon(QStringLiteral("nav_settings_1")));
        ui.navSettings->setIconSize(navIconSize);
    }
    if (ui.navStats) {
        ui.navStats->setText(QStringLiteral("统计"));
        ui.navStats->setIcon(loadNavIcon(QStringLiteral("nav_stats")));
        ui.navStats->setIconSize(navIconSize);
    }
    if (ui.navHelp) {
        ui.navHelp->setText(QStringLiteral("帮助中心"));
        ui.navHelp->setIcon(loadNavIcon(QStringLiteral("nav_help")));
        ui.navHelp->setIconSize(navIconSize);
    }
    
    // Header toolbar icons
    
    // Sidebar bottom
    if (ui.btnDisconnect) 
    {
        ui.btnDisconnect->setIcon(loadIcon("btn_disconnect"));
        ui.btnDisconnect->setIconSize(QSize(20, 20));
    }
    if (ui.logoIcon) {
        ui.logoIcon->setFixedSize(QSize(40, 40));
        ui.logoIcon->setPixmap(QPixmap());
    }
    if (ui.btnStartProxy)
    {
        connect(ui.btnStartProxy, &QPushButton::clicked, this, &v2raycpp::onStartClicked);
    }

    if (ui.toolRefresh) {
        ui.toolRefresh->setToolTip(QStringLiteral("刷新全部节点延迟与地区信息"));
        connect(ui.toolRefresh, &QPushButton::clicked, this, &v2raycpp::onRefreshLatencyClicked);
    }

    if (ui.navSettings)
    {
        connect(ui.navSettings, &QPushButton::clicked, this, &v2raycpp::onSettingsClicked);
    }

    if (ui.btnWindowClose) {
        ui.btnWindowClose->setToolTip(QStringLiteral("关闭窗口"));
        connect(ui.btnWindowClose, &QPushButton::clicked, this, &QWidget::close);
    }

    if (ui.btnDisconnect)
    {
        connect(ui.btnDisconnect, &QPushButton::clicked, this, &v2raycpp::onDisconnectAccountClicked);
    }

    // Connect search box
    if (ui.searchBar)
    {
        connect(ui.searchBar, &QLineEdit::textChanged, this, &v2raycpp::onSearchTextChanged);
    }

    if (ui.protocolFilterBar) {
        ui.protocolFilterBar->setAttribute(Qt::WA_StyledBackground, true);
    }

    m_protocolFilterGroup = new QButtonGroup(this);
    m_protocolFilterGroup->setExclusive(true);
    if (ui.filterAll)
        m_protocolFilterGroup->addButton(ui.filterAll);
    if (ui.filterVmess)
        m_protocolFilterGroup->addButton(ui.filterVmess);
    if (ui.filterTrojan)
        m_protocolFilterGroup->addButton(ui.filterTrojan);
    if (ui.filterVless)
        m_protocolFilterGroup->addButton(ui.filterVless);
    connect(m_protocolFilterGroup, &QButtonGroup::buttonClicked, this, [this](QAbstractButton*) {
        updateServerCardFilter();
    });
}

void v2raycpp::initConnections()
{
    // Connect CoreManager signals
    connect(m_coreManager.get(), &CoreManager::logOutput, 
            this, &v2raycpp::onLogOutput);
    connect(m_coreManager.get(), &CoreManager::errorOutput, 
            this, &v2raycpp::onErrorOutput);
    connect(m_coreManager.get(), &CoreManager::statusChanged, 
            this, &v2raycpp::onStatusChanged);
    connect(m_coreManager.get(), &CoreManager::processFinished, 
            this, &v2raycpp::onProcessFinished);
}

void v2raycpp::updateUIStatus()
{
    // Update tray icon status
    if (m_trayIcon)
    {
        switch (m_currentStatus)
        {
            case CoreStatus::Running:
                m_trayIcon->setStatus(ETrayStatus::Running);
                break;
            case CoreStatus::Stopped:
                m_trayIcon->setStatus(ETrayStatus::Stopped);
                break;
            default:
                m_trayIcon->setStatus(ETrayStatus::Disconnected);
                break;
        }
    }
    
    // Update UI elements based on current status
    QString statusText;
    
    switch (m_currentStatus)
    {
        case CoreStatus::Stopped:
            statusText = "Stopped";
            break;
        case CoreStatus::Starting:
            statusText = "Starting";
            break;
        case CoreStatus::Running:
            statusText = "Running";
            break;
        case CoreStatus::Stopping:
            statusText = "Stopping";
            break;
    }
    
    // Update status bar
    if (ui.statConnLabel)
    {
        ui.statConnLabel->setText(statusText);
    }
    
    // Update startProxyBtn text
    if (ui.btnStartProxy)
    {
        if (m_currentStatus == CoreStatus::Running)
        {
            ui.btnStartProxy->setText(QStringLiteral("停止系统代理"));
        }
        else
        {
            ui.btnStartProxy->setText(QStringLiteral("启动系统代理"));
        }
    }
    
    // Update speed labels to show 0 when stopped
    if (m_currentStatus == CoreStatus::Stopped)
    {
        if (ui.statSpeedDown)
        {
            ui.statSpeedDown->setText("0 KB/s");
        }
        if (ui.statSpeedUp)
        {
            ui.statSpeedUp->setText("0 KB/s");
        }
        if (ui.statIP)
        {
            ui.statIP->setText("IP: --");
        }
    }

}

void v2raycpp::updateStatusBar()
{
    // Update current node label
    if (ui.statConnLabel)
    {
        if (m_currentProfile.isValid())
        {
            QString nodeInfo = QString("%1 - %2:%3")
                .arg(QString::fromStdString(m_currentProfile.getRemark().empty() ? 
                      m_currentProfile.getAddress() : m_currentProfile.getRemark()))
                .arg(QString::fromStdString(m_currentProfile.getAddress()))
                .arg(m_currentProfile.getPort());
            ui.statConnLabel->setText(nodeInfo);
        }
        else
        {
            ui.statConnLabel->setText("No Server");
        }
    }
    
    if (ui.statUptime) {
        ui.statUptime->setText(formatFooterUptime(m_startTime, m_currentStatus));
    }

    syncServerCardSelectionWithCurrentProfile();
}

void v2raycpp::loadConfig()
{
    AppConfig::instance().load();

    if (AppConfig::instance().getCoreConfigPath().isEmpty()) {
        AppConfig::instance().setCoreConfigPath(AppConfig::instance().getDefaultConfigPath());
    }

    QString configPath = AppConfig::instance().getConfigPath();
    QString configFile = configPath + "/config.json";
    qDebug() << "[DEBUG] AppConfig.getConfigPath() =" << configPath;
    qDebug() << "[DEBUG] Trying to open config file:" << configFile;

    QString err;
    if (!XrayConfigStore::loadServerProfiles(configFile, m_serverProfiles, &err)) {
        qWarning() << "[DEBUG] loadServerProfiles failed:" << err;
        return;
    }

    for (const auto& profile : m_serverProfiles) {
        addServerToList(profile);
    }

    syncServerCardSelectionWithCurrentProfile();
}


void v2raycpp::saveConfig()
{
    AppConfig::instance().save();

    QString configPath = AppConfig::instance().getConfigPath();
    QString configFile = configPath + "/config.json";

    QString err;
    if (!XrayConfigStore::saveServerProfiles(configFile, m_serverProfiles, &err)) {
        qWarning() << "saveServerProfiles:" << err;
    }
}


// ==================== Slot Implementations ====================

void v2raycpp::onStartClicked()
{
    if (m_currentStatus == CoreStatus::Running)
    {
        QMessageBox::information(this, "Info", "Already running");
        return;
    }
    
    // Check if we have a valid profile
    if (!m_currentProfile.isValid())
    {
        // Try to get selected profile from list
        // Get from grid - TODO
    }
    if (m_serverProfiles.empty()) {
        QMessageBox::warning(this, "Warning", "Please add a server first");
        return;
    }
    m_currentProfile = m_serverProfiles[0];

    QString configPath = AppConfig::instance().getCoreConfigPath();
    if (!ConfigGenerator::writeStartupCoreConfig(m_currentProfile, configPath)) {
        QMessageBox::critical(this, "Error", "Failed to generate config");
        return;
    }
    
    // Start core
    if (m_coreManager->startCore(configPath))
    {
        // Set system proxy
        int localPort = AppConfig::instance().getLocalPort();
        m_sysProxyHandler->setProxy(QString("127.0.0.1:%1").arg(localPort).toStdString());
        
        // Record start time
        m_startTime = QDateTime::currentDateTime();
        
        // Start traffic statistics timer
        startStatsTimer();
        
        // Update UI
        updateStatusBar();
        
        // statusBar()->showMessage() removed - using statusLabel instead
    }
    else
    {
        QMessageBox::critical(this, "Error", "Failed to start core");
    }
}

void v2raycpp::onStopClicked()
{
    if (m_currentStatus == CoreStatus::Stopped)
    {
        QMessageBox::information(this, "Info", "Already stopped");
        return;
    }

    // 停止核心
    if (m_coreManager->stopCore())
    {
        // 停止流量统计计时器
        stopStatsTimer();

        // 停止自动重连计时器（用户手动停止）
        stopReconnectTimer();

        // 清除系统代理
        m_sysProxyHandler->clearProxy();

        // 重置启动时间
        m_startTime = QDateTime();

        // 更新状态栏
        updateStatusBar();
    }
    else
    {
        QMessageBox::warning(this, "Warning", "Failed to stop core");
    }
}

void v2raycpp::onDisconnectAccountClicked()
{
    if (m_coreManager && m_currentStatus != CoreStatus::Stopped) {
        m_coreManager->stopCore();
    }
    stopStatsTimer();
    stopReconnectTimer();
    if (m_sysProxyHandler) {
        m_sysProxyHandler->clearProxy();
    }
    m_startTime = QDateTime();
    updateUIStatus();
    updateStatusBar();
    emit accountLogoutRequested();
}

void v2raycpp::onImportClicked()
{
    // Get text from clipboard
    QClipboard* clipboard = QApplication::clipboard();
    QString text = clipboard->text();

    if (text.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Clipboard is empty");
        return;
    }

    ProfileItem profile;
    const auto pr = ProfileManager::tryParseProfileFromUrl(text, profile);
    if (pr == ProfileManager::UrlParseResult::Ok) {
        m_serverProfiles.push_back(profile);
        addServerToList(profile);
        m_currentProfile = m_serverProfiles.back();
        updateStatusBar();
    } else if (pr == ProfileManager::UrlParseResult::UnsupportedProtocol) {
        QMessageBox::information(this, "Info", "VMess/VLESS URL import is not supported yet");
    } else {
        QMessageBox::warning(this, "Warning", "Failed to parse URL");
    }
}

void v2raycpp::onSettingsClicked()
{
    show();
    activateWindow();
    QMessageBox::information(this, "Settings", "Settings dialog not implemented yet");
}

void v2raycpp::onDeleteServerClicked()
{
    // TODO: implement with grid
    QMessageBox::information(this, "Info", "Use grid to select server");
}


void v2raycpp::onEditServerClicked()
{
    // TODO: implement with grid
    QMessageBox::information(this, "Info", "Use grid to select server");
}


void v2raycpp::onLogOutput(const QString& log)
{
    // Log to debug output
    qDebug() << "[Core]" << log;
}

void v2raycpp::onErrorOutput(const QString& error)
{
    // Log error
    qWarning() << "[Core Error]" << error;
    // statusBar()->showMessage() removed - using statusLabel instead
}

void v2raycpp::onStatusChanged(CoreStatus status)
{
    m_currentStatus = status;
    updateUIStatus();
    
    // Handle specific status changes
    switch (status)
    {
        case CoreStatus::Running:
            // Stop reconnect timer when running
            stopReconnectTimer();
            // statusBar()->showMessage() removed - using statusLabel instead
            break;
        case CoreStatus::Stopped:
            // Clear proxy when stopped
            m_sysProxyHandler->clearProxy();
            // Start auto reconnect if enabled and was previously running
            if (m_autoReconnect && m_currentProfile.isValid())
            {
                startReconnectTimer();
            }
            // statusBar()->showMessage() removed - using statusLabel instead
            break;
        default:
            break;
    }
    
    updateStatusBar();
}

void v2raycpp::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString message = QString("Process exited with code: %1").arg(exitCode);
    qDebug() << message;
    
    m_currentStatus = CoreStatus::Stopped;
    updateUIStatus();
    
    // Clear system proxy
    m_sysProxyHandler->clearProxy();
    
    // Reset start time
    m_startTime = QDateTime();
    updateStatusBar();
    
    if (exitStatus == QProcess::CrashExit)
    {
        QMessageBox::warning(this, "Warning", 
            QString("Core process crashed with code: %1").arg(exitCode));
    }
    
    // Start auto reconnect if enabled and was previously connected
    if (m_autoReconnect && m_currentProfile.isValid())
    {
        startReconnectTimer();
    }
}

// ==================== Tray Icon Slot Implementations ====================

void v2raycpp::onShowWindowFromTray()
{
    show();
    activateWindow();
}

void v2raycpp::onSettingsFromTray()
{
    show();
    activateWindow();
    onSettingsClicked();
}

void v2raycpp::onExitFromTray()
{
    // Save config before exit
    saveConfig();
    
    // Stop core if running
    if (m_coreManager && m_coreManager->isRunning())
    {
        m_coreManager->stopCore();
    }
    
    // Clear system proxy
    if (m_sysProxyHandler)
    {
        m_sysProxyHandler->clearProxy();
    }
    
    // Hide tray icon
    if (m_trayIcon)
    {
        m_trayIcon->hide();
    }
    
    QApplication::quit();
}

void v2raycpp::onTrayDoubleClicked()
{
    if (isVisible())
    {
        hide();
    }
    else
    {
        show();
        activateWindow();
    }
}

void v2raycpp::onEnableSystemProxy()
{
    if (m_currentStatus == CoreStatus::Running)
    {
        int localPort = AppConfig::instance().getLocalPort();
        m_sysProxyHandler->setProxy(QString("127.0.0.1:%1").arg(localPort).toStdString());
        // statusBar()->showMessage() removed - using statusLabel instead
    }
    else
    {
        QMessageBox::warning(this, "Warning", "Please start proxy first");
    }
}

void v2raycpp::onDisableSystemProxy()
{
    m_sysProxyHandler->clearProxy();
    // statusBar()->showMessage() removed - using statusLabel instead
}

void v2raycpp::onRefreshLatencyClicked()
{
    const int n = static_cast<int>(m_serverProfiles.size());
    for (int i = 0; i < n; ++i) {
        if (i >= m_serverCards.size())
            break;
        SimpleCard* card = m_serverCards.at(i).data();
        if (card)
            startCardServerProbe(card, i);
    }
}

// ==================== Helper Functions ====================

void v2raycpp::addServerToList(const ProfileItem& profile)
{
    QString label = QString::fromStdString(profile.getRemark().empty() ?
                                           profile.getAddress() : profile.getRemark());
    QString protocol;
    bool isConnected = (m_currentStatus == CoreStatus::Running &&
                        profile.getAddress() == m_currentProfile.getAddress());

    switch (profile.getConfigType()) {
        case EConfigType::VMess: protocol = "VMess"; break;
        case EConfigType::VLESS: protocol = "VLESS"; break;
        case EConfigType::Trojan: protocol = "Trojan"; break;
        case EConfigType::Shadowsocks: protocol = "Shadowsocks"; break;
        default: protocol = "Unknown"; break;
    }

    // 查找 profile 在 m_serverProfiles 的索引
    int serverIndex = -1;
    for (int i = 0; i < (int)m_serverProfiles.size(); ++i) {
        const ProfileItem &p = m_serverProfiles[i];
        if (p.getAddress() == profile.getAddress()
            && p.getPort() == profile.getPort()
            && p.getRemark() == profile.getRemark()) {
            serverIndex = i;
            break;
        }
    }
    if (serverIndex == -1) serverIndex = static_cast<int>(m_serverProfiles.size()) - 1;

    // 只向 UI 的 gridLayout 添加卡片
    if (ui.formLayout)
    {
        addCardToGrid(label, protocol, -1, isConnected, serverIndex);
    }
}

void v2raycpp::addCardToGrid(const QString& title, const QString& protocol, int latency, bool connected, int serverIndex)
{
    if (!ui.formLayout) return;

    // 创建卡片并设置固定大小
    SimpleCard* card = new SimpleCard(this);
    card->setNodeInfo(title, latency, protocol, connected);
    card->setFlag(QString());
    card->setFixedSize(420, 200);

    connect(card, &SimpleCard::clicked, this, [this, serverIndex]() {
        if (serverIndex >= 0 && serverIndex < (int)m_serverProfiles.size()) {
            m_currentProfile = m_serverProfiles[serverIndex];
            updateStatusBar();
            qDebug() << "Selected profile index:" << serverIndex;
        } else {
            qDebug() << "Card clicked (no bound server):" << serverIndex;
        }
    });

    connect(card, &SimpleCard::toggled, this, [this, serverIndex](bool on) {
        if (serverIndex >= 0 && serverIndex < (int)m_serverProfiles.size()) {
            m_currentProfile = m_serverProfiles[serverIndex];
            if (on) onStartClicked(); else onStopClicked();
        } else {
            qDebug() << "Toggled (no bound server):" << on;
        }
    });

    if (serverIndex >= 0 && serverIndex < static_cast<int>(m_serverProfiles.size())) {
        if (m_serverCards.size() <= serverIndex)
            m_serverCards.resize(serverIndex + 1);
        m_serverCards[serverIndex] = card;
    }

    const int columns = 2;
    // 尝试把新卡片放入最后一行（如果最后一行有容器且未满）
    QWidget* targetContainer = nullptr;
    bool createdNewRow = false;
    int rows = ui.formLayout->rowCount();
    if (rows > 0) {
        QLayoutItem* lastField = ui.formLayout->itemAt(rows - 1, QFormLayout::FieldRole);
        if (lastField && lastField->widget()) {
            QWidget* lastWidget = lastField->widget();
            QLayout* lay = lastWidget->layout();
            if (lay) {
                int childWidgets = 0;
                for (int i = 0; i < lay->count(); ++i) {
                    QLayoutItem* it = lay->itemAt(i);
                    if (it && it->widget()) ++childWidgets;
                }
                if (childWidgets < columns) {
                    targetContainer = lastWidget;
                }
            }
        }
    }

    // 如果没有合适的容器，则新建一行容器
    if (!targetContainer) {
        targetContainer = new QWidget();
        QHBoxLayout* hl = new QHBoxLayout(targetContainer);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(12);
        createdNewRow = true;
    }

    // 将卡片加入容器
    if (QLayout* tl = targetContainer->layout()) {
        tl->addWidget(card);
    } else {
        // 保底，直接把卡片作为子控件（不太可能走到这里）
        card->setParent(targetContainer);
    }

    // 如果是新建容器，则把它作为新行添加到 formLayout
    if (createdNewRow) {
        ui.formLayout->addRow(targetContainer);
    }

    // 更新几何与重绘，确保布局生效
    if (ui.formLayout->parentWidget()) ui.formLayout->parentWidget()->updateGeometry();
    this->update();

    if (serverIndex >= 0 && serverIndex < static_cast<int>(m_serverProfiles.size()))
        startCardServerProbe(card, serverIndex);

    updateServerCardFilter();
}

void v2raycpp::updateServerCardFilter()
{
    const int n = static_cast<int>(m_serverProfiles.size());
    const QString q = ui.searchBar ? ui.searchBar->text().trimmed().toLower() : QString();

    auto matchesProtocol = [this](const ProfileItem& p) -> bool {
        if (ui.filterAll && ui.filterAll->isChecked())
            return true;
        if (ui.filterVmess && ui.filterVmess->isChecked())
            return p.getConfigType() == EConfigType::VMess;
        if (ui.filterTrojan && ui.filterTrojan->isChecked())
            return p.getConfigType() == EConfigType::Trojan;
        if (ui.filterVless && ui.filterVless->isChecked())
            return p.getConfigType() == EConfigType::VLESS;
        return true;
    };

    for (int i = 0; i < n; ++i) {
        SimpleCard* card = (i < m_serverCards.size()) ? m_serverCards.at(i).data() : nullptr;
        if (!card)
            continue;

        const ProfileItem& pr = m_serverProfiles[static_cast<size_t>(i)];
        if (!matchesProtocol(pr)) {
            card->setVisible(false);
            continue;
        }

        if (q.isEmpty()) {
            card->setVisible(true);
            continue;
        }

        const QString blob = QStringLiteral("%1 %2 %3")
                                 .arg(QString::fromStdString(pr.getRemark()))
                                 .arg(QString::fromStdString(pr.getAddress()))
                                 .arg(protocolSearchKeyword(pr.getConfigType()))
                                 .toLower();
        card->setVisible(blob.contains(q));
    }
}

void v2raycpp::applyServerCardSelection(int serverIndex)
{
    for (int i = 0; i < m_serverCards.size(); ++i) {
        SimpleCard* c = m_serverCards.at(i).data();
        if (!c)
            continue;
        c->setSelected(serverIndex >= 0 && i == serverIndex);
    }
}

void v2raycpp::syncServerCardSelectionWithCurrentProfile()
{
    if (!m_currentProfile.isValid()) {
        applyServerCardSelection(-1);
        return;
    }
    int idx = -1;
    for (int i = 0; i < static_cast<int>(m_serverProfiles.size()); ++i) {
        const ProfileItem& p = m_serverProfiles[static_cast<size_t>(i)];
        if (p.getAddress() == m_currentProfile.getAddress()
            && p.getPort() == m_currentProfile.getPort()
            && p.getRemark() == m_currentProfile.getRemark()) {
            idx = i;
            break;
        }
    }
    applyServerCardSelection(idx);
}

void v2raycpp::startCardServerProbe(SimpleCard* card, int serverIndex)
{
    if (!card || !m_serverProbe || serverIndex < 0 || serverIndex >= static_cast<int>(m_serverProfiles.size())) {
        return;
    }

    const ProfileItem& pr = m_serverProfiles[static_cast<size_t>(serverIndex)];
    const QString host = QString::fromStdString(pr.getAddress());
    const int port = pr.getPort();
    if (host.isEmpty() || port <= 0) {
        return;
    }

    card->setLatencyValue(kLatencyProbePending);
    card->setLocationInfo(QStringLiteral("定位中…"), QString());
    m_serverProbe->startProbe(serverIndex, host, port);
}

void v2raycpp::onProbeLatency(int serverIndex, int latencyMs)
{
    if (serverIndex >= 0 && serverIndex < static_cast<int>(m_serverProfiles.size())) {
        m_serverProfiles[static_cast<size_t>(serverIndex)].setLatency(latencyMs);
    }
    if (serverIndex >= 0 && serverIndex < m_serverCards.size()) {
        if (SimpleCard* c = m_serverCards[serverIndex].data()) {
            c->setLatencyValue(latencyMs);
        }
    }
}

void v2raycpp::onProbeGeo(int serverIndex, const QString& primaryLine, const QString& secondaryLine,
                          const QString& country, const QString& countryCode)
{
    if (serverIndex >= 0 && serverIndex < static_cast<int>(m_serverProfiles.size())) {
        if (!country.isEmpty()) {
            m_serverProfiles[static_cast<size_t>(serverIndex)].setCountry(country.toStdString());
            m_serverProfiles[static_cast<size_t>(serverIndex)].setCountryCode(countryCode.toStdString());
        }
    }
    if (serverIndex >= 0 && serverIndex < m_serverCards.size()) {
        if (SimpleCard* c = m_serverCards[serverIndex].data()) {
            c->setLocationInfo(primaryLine, secondaryLine);
        }
    }
}

ProfileItem v2raycpp::getSelectedProfile() const
{
    if (m_serverProfiles.size() > 0) return m_serverProfiles[0];
    return ProfileItem();
}




void v2raycpp::startStatsTimer()
{
    if (m_trafficStats) {
        m_trafficStats->start();
    }
}

void v2raycpp::stopStatsTimer()
{
    if (m_trafficStats) {
        m_trafficStats->stop();
    }
}

void v2raycpp::startReconnectTimer()
{
    if (m_reconnectController) {
        m_reconnectController->arm(5000);
    }
}

void v2raycpp::stopReconnectTimer()
{
    if (m_reconnectController) {
        m_reconnectController->disarm();
    }
}

void v2raycpp::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text);
    updateServerCardFilter();
}