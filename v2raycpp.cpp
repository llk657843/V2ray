#include "v2raycpp.h"
#include <QMouseEvent>
#include <QIcon>
#include <QSize>
#include <QPixmap>
#include <QPainter>
#include "TrayIcon.h"
#include "TrojanFmt.h"
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
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QElapsedTimer>
#include <QThread>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include <QPointer>
#include <QUrl>

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

int tcpConnectLatencyMs(const QString& host, int port)
{
    QElapsedTimer timer;
    timer.start();
    QTcpSocket socket;
    socket.connectToHost(host, port);
    if (socket.waitForConnected(3000))
        return static_cast<int>(timer.elapsed());
    return -1;
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

    m_geoNetwork = new QNetworkAccessManager(this);
    
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
    ui.logoIcon->setFixedSize(QSize(40, 40));
        // Server grid is created in initServerGrid()\n    // Old serverList not used\n    \n    // Connect new UI buttons
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

    // btnClose not in new UI

    if (ui.btnDisconnect)
    {
        connect(ui.btnDisconnect, &QPushButton::clicked, this, &v2raycpp::onStopClicked);
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

bool v2raycpp::generateCoreConfig(const ProfileItem& profile)
{
    // Generate Xray core config JSON from profile
    QJsonObject config;
    
    // Log settings
    QJsonObject logObj;
    logObj["loglevel"] = "warning";
    config["log"] = logObj;
    
    // Inbounds (local proxy)
    QJsonArray inbounds;
    QJsonObject inbound;
    inbound["tag"] = "socks-inbound";
    inbound["protocol"] = "socks";
    
    QJsonObject socksSettings;
    socksSettings["auth"] = "noauth";
    socksSettings["udp"] = true;
    socksSettings["ip"] = "127.0.0.1";
    
    QJsonArray accounts;
    socksSettings["accounts"] = accounts;
    
    inbound["settings"] = socksSettings;
    
    inbound["listen"] = "127.0.0.1";
    auto port = AppConfig::instance().getLocalPort();
    inbound["port"] = port;
    
    inbounds.append(inbound);
    config["inbounds"] = inbounds;
    
    // Outbounds (remote proxy)
    QJsonArray outbounds;
    QJsonObject outbound;
    outbound["tag"] = "proxy";
    
    // Determine protocol based on profile
    QString protocol;
    switch (profile.getConfigType())
    {
        case EConfigType::VMess:
            protocol = "vmess";
            break;
        case EConfigType::VLESS:
            protocol = "vless";
            break;
        case EConfigType::Trojan:
            protocol = "trojan";
            break;
        case EConfigType::Shadowsocks:
            protocol = "shadowsocks";
            break;
        default:
            protocol = "trojan";
    }
    
    outbound["protocol"] = protocol.toStdString().c_str();
    
    // Build settings based on config type
    QJsonObject settings;
    
    if (protocol == "trojan")
    {
        // Use servers array format like V2RayN
        QJsonArray servers;
        QJsonObject server;
        server["address"] = profile.getAddress().c_str();
        server["password"] = profile.getPassword().c_str();
        server["port"] = profile.getPort();
        server["level"] = 1;
        servers.append(server);

        QJsonObject trojanSettings;
        trojanSettings["servers"] = servers;

        // TLS settings
        if (profile.getSecurity() == "tls")
        {
            QJsonObject tlsSettings;
            tlsSettings["serverName"] = profile.getSni().c_str();
            tlsSettings["allowInsecure"] = true;

            if (!profile.getFingerprint().empty())
            {
                tlsSettings["fingerprint"] = profile.getFingerprint().c_str();
            }
            else
            {
                tlsSettings["fingerprint"] = "random";
            }

            QJsonArray alpn;
            alpn.append("h2");
            alpn.append("http/1.1");
            tlsSettings["alpn"] = alpn;

            QJsonObject streamSettings;
            streamSettings["network"] = profile.getNetwork().c_str();
            streamSettings["security"] = "tls";
            streamSettings["tlsSettings"] = tlsSettings;

            QJsonObject mux;
            mux["enabled"] = false;
            mux["concurrency"] = -1;
            outbound["mux"] = mux;

            outbound["streamSettings"] = streamSettings;
        }

        outbound["settings"] = trojanSettings;
    }
    
    outbounds.append(outbound);
    
    // Direct outbound
    QJsonObject directOutbound;
    directOutbound["tag"] = "direct";
    directOutbound["protocol"] = "freedom";
    outbounds.append(directOutbound);
    
    // Block outbound
    QJsonObject blockOutbound;
    blockOutbound["tag"] = "block";
    blockOutbound["protocol"] = "blackhole";
    outbounds.append(blockOutbound);
    
    config["outbounds"] = outbounds;
    
    // Routing
    QJsonObject routing;
    routing["domainStrategy"] = "IPIfNonMatch";
    routing["mode"] = "proxy";
    
    QJsonObject rules;
    rules["type"] = "field";
    rules["outboundTag"] = "proxy";
    rules["domain"] = QJsonArray::fromStringList({"geosite:category-ads-all"});
    
    QJsonArray ruleArray;
    ruleArray.append(rules);
    routing["rules"] = ruleArray;
    
    config["routing"] = routing;
    
    // Write to config file
    QJsonDocument doc(config);
    QString configPath = AppConfig::instance().getCoreConfigPath();
    
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open config file for writing:" << configPath;
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

void v2raycpp::loadConfig()
{
    // Load application configuration
    AppConfig::instance().load();

    // Initialize core config path if not set
    if (AppConfig::instance().getCoreConfigPath().isEmpty()) {
        AppConfig::instance().setCoreConfigPath(AppConfig::instance().getDefaultConfigPath());
    }

    // Load server list from Xray config.json file
    // config.json format: { inbounds: [...], outbounds: [...], routing: {...} }
    // Server info is stored in outbounds array, where each outbound represents a server
    QString configPath = AppConfig::instance().getConfigPath();
    QString configFile = configPath + "/config.json";
    qDebug() << "[DEBUG] AppConfig.getConfigPath() =" << configPath;
    qDebug() << "[DEBUG] Trying to open config file:" << configFile;
    
    QFile file(configFile);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[DEBUG] No config.json found at path:" << configFile;
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    qDebug() << "[DEBUG] config.json content size:" << data.size();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "[DEBUG] Failed to parse config.json:" << parseError.errorString();
        return;
    }
    
    if (!doc.isObject())
    {
        qWarning() << "[DEBUG] config.json is not a valid JSON object";
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray outbounds = root["outbounds"].toArray();
    
    for (int i = 0; i < outbounds.size(); ++i)
    {
        QJsonObject outbound = outbounds[i].toObject();
        QString protocol = outbound["protocol"].toString();
        QString tag = outbound["tag"].toString();
        
        // Skip non-proxy outbounds (direct, block, dns, etc.)
        if (tag == "direct" || tag == "block" || tag == "dns-outbound" || tag.isEmpty())
        {
            continue;
        }
        
        // Parse server settings based on protocol
        QJsonObject settings = outbound["settings"].toObject();
        QJsonObject streamSettings = outbound["streamSettings"].toObject();
        
        ProfileItem profile;
        profile.setRemark(tag.toStdString()); // Use tag as remark
        
        if (protocol == "trojan")
        {
            QJsonArray servers = settings["servers"].toArray();
            if (servers.size() > 0)
            {
                QJsonObject server = servers[0].toObject();
                profile.setAddress(server["address"].toString().toStdString());
                profile.setPort(server["port"].toInt());
                profile.setPassword(server["password"].toString().toStdString());
                
                // Load flow (for XTLS modes like xtls-rprx-vision)
                QString flow = server["flow"].toString();
                if (!flow.isEmpty())
                {
                    profile.setFlow(flow.toStdString());
                }
                
                profile.setConfigType(EConfigType::Trojan);
            }
        }
        else if (protocol == "vmess")
        {
            QJsonArray vnext = settings["vnext"].toArray();
            if (vnext.size() > 0)
            {
                QJsonObject server = vnext[0].toObject();
                profile.setAddress(server["address"].toString().toStdString());
                profile.setPort(server["port"].toInt());
                
                QJsonArray users = server["users"].toArray();
                if (users.size() > 0)
                {
                    QJsonObject user = users[0].toObject();
                    profile.setUserId(user["id"].toString().toStdString());
                    profile.setSecurity(user["security"].toString().toStdString());
                }
                profile.setConfigType(EConfigType::VMess);
            }
        }
        else if (protocol == "vless")
        {
            QJsonArray vnext = settings["vnext"].toArray();
            if (vnext.size() > 0)
            {
                QJsonObject server = vnext[0].toObject();
                profile.setAddress(server["address"].toString().toStdString());
                profile.setPort(server["port"].toInt());
                
                QJsonArray users = server["users"].toArray();
                if (users.size() > 0)
                {
                    QJsonObject user = users[0].toObject();
                    profile.setUserId(user["id"].toString().toStdString());
                    
                    // Load flow (for XTLS modes like xtls-rprx-vision)
                    QString flow = user["flow"].toString();
                    if (!flow.isEmpty())
                    {
                        profile.setFlow(flow.toStdString());
                    }
                }
                profile.setConfigType(EConfigType::VLESS);
            }
        }
        else
        {
            // Unsupported protocol, skip
            continue;
        }
        
        // Parse stream settings (common for all protocols)
        QString network = streamSettings["network"].toString("tcp");
        QString security = streamSettings["security"].toString("");
        profile.setNetwork(network.toStdString());
        profile.setSecurity(security.toStdString());
        
        // TLS settings
        if (security == "tls")
        {
            QJsonObject tlsSettings = streamSettings["tlsSettings"].toObject();
            profile.setSni(tlsSettings["serverName"].toString().toStdString());
            profile.setAllowInsecure(tlsSettings["allowInsecure"].toBool(false));
            
            QJsonArray alpn = tlsSettings["alpn"].toArray();
            if (alpn.size() > 0)
            {
                profile.setAlpn(alpn[0].toString().toStdString());
            }
            
            QString fingerprint = tlsSettings["fingerprint"].toString();
            if (!fingerprint.isEmpty())
            {
                profile.setFingerprint(fingerprint.toStdString());
            }
        }
        
        // Reality settings
        if (security == "reality")
        {
            QJsonObject realitySettings = streamSettings["realitySettings"].toObject();
            profile.setSni(realitySettings["serverName"].toString().toStdString());
            
            QString fingerprint = realitySettings["fingerprint"].toString();
            if (!fingerprint.isEmpty())
            {
                profile.setFingerprint(fingerprint.toStdString());
            }
            
            // Load Reality-specific fields
            QString publicKey = realitySettings["publicKey"].toString();
            if (!publicKey.isEmpty())
            {
                profile.setPublicKey(publicKey.toStdString());
            }
            
            QString shortId = realitySettings["shortId"].toString();
            if (!shortId.isEmpty())
            {
                profile.setShortId(shortId.toStdString());
            }
            
            QString spiderX = realitySettings["spiderX"].toString();
            if (!spiderX.isEmpty())
            {
                profile.setSpiderX(spiderX.toStdString());
            }
        }
        
        // Network-specific settings (WebSocket, gRPC, HTTP/2, etc.)
        if (network == "ws")
        {
            // WebSocket settings
            QJsonObject wsSettings = streamSettings["wsSettings"].toObject();
            QString wsPath = wsSettings["path"].toString();
            QString wsHost = wsSettings["host"].toString();
            // Note: ProfileItem doesn't have path/host fields, would need to extend
        }
        else if (network == "grpc")
        {
            // gRPC settings
            QJsonObject grpcSettings = streamSettings["grpcSettings"].toObject();
            QString grpcServiceName = grpcSettings["serviceName"].toString();
            QString grpcAuthority = grpcSettings["authority"].toString();
            // Note: ProfileItem doesn't have path/host fields, would need to extend
        }
        else if (network == "h2")
        {
            // HTTP/2 settings
            QJsonObject httpSettings = streamSettings["httpSettings"].toObject();
            QString httpPath = httpSettings["path"].toString();
            // Note: ProfileItem doesn't have path/host fields, would need to extend
        }
        
        if (profile.isValid())
        {
            m_serverProfiles.push_back(profile);
            addServerToList(profile);
        }
        else {
            qDebug() << "[DEBUG] Skipping invalid profile at index" << i
                     << "addr:" << QString::fromStdString(profile.getAddress())
                     << "port:" << profile.getPort()
                     << "remark:" << QString::fromStdString(profile.getRemark());
        }
    }
    
    qDebug() << "[DEBUG] Loaded" << m_serverProfiles.size() << "valid servers from config.json";

    syncServerCardSelectionWithCurrentProfile();
}

void v2raycpp::saveConfig()
{
    // Save application configuration
    AppConfig::instance().save();
    
    // Save server list to config.json in Xray format
    // config.json format: { inbounds: [...], outbounds: [...], routing: {...} }
    QString configPath = AppConfig::instance().getConfigPath();
    QString configFile = configPath + "/config.json";
    
    // Read existing config.json to preserve inbounds, routing, etc.
    QJsonObject root;
    QFile readFile(configFile);
    if (readFile.open(QIODevice::ReadOnly))
    {
        QJsonDocument existingDoc = QJsonDocument::fromJson(readFile.readAll());
        readFile.close();
        if (existingDoc.isObject())
        {
            root = existingDoc.object();
        }
    }
    
    // Build outbounds array from server profiles
    QJsonArray outbounds;
    
    for (int i = 0; i < m_serverProfiles.size(); ++i)
    {
        const ProfileItem& profile = m_serverProfiles[i];
        
        QJsonObject outbound;
        outbound["tag"] = QString::fromStdString(profile.getRemark().empty() ? 
                          QString("proxy-%1").arg(i + 1).toStdString() : profile.getRemark());
        
        // Set protocol based on config type
        QString protocol;
        switch (profile.getConfigType())
        {
            case EConfigType::VMess:
                protocol = "vmess";
                break;
            case EConfigType::VLESS:
                protocol = "vless";
                break;
            case EConfigType::Trojan:
                protocol = "trojan";
                break;
            case EConfigType::Shadowsocks:
                protocol = "shadowsocks";
                break;
            default:
                protocol = "trojan";
        }
        outbound["protocol"] = protocol;
        
        // Build settings based on protocol
        QJsonObject settings;
        
        if (protocol == "trojan")
        {
            QJsonArray servers;
            QJsonObject server;
            server["address"] = QString::fromStdString(profile.getAddress());
            server["port"] = profile.getPort();
            server["password"] = QString::fromStdString(profile.getPassword());
            
            // Save flow (for XTLS modes)
            QString flow = QString::fromStdString(profile.getFlow());
            if (!flow.isEmpty())
            {
                server["flow"] = flow;
            }
            
            server["ota"] = false;
            server["level"] = 1;
            servers.append(server);
            settings["servers"] = servers;
        }
        else if (protocol == "vmess")
        {
            QJsonArray vnext;
            QJsonObject server;
            server["address"] = QString::fromStdString(profile.getAddress());
            server["port"] = profile.getPort();
            
            QJsonArray users;
            QJsonObject user;
            user["id"] = QString::fromStdString(profile.getUserId());
            user["alterId"] = 0;
            user["security"] = QString::fromStdString(profile.getSecurity().empty() ? "auto" : profile.getSecurity());
            user["email"] = "user@v2raycpp";
            users.append(user);
            server["users"] = users;
            
            vnext.append(server);
            settings["vnext"] = vnext;
        }
        else if (protocol == "vless")
        {
            QJsonArray vnext;
            QJsonObject server;
            server["address"] = QString::fromStdString(profile.getAddress());
            server["port"] = profile.getPort();
            
            QJsonArray users;
            QJsonObject user;
            user["id"] = QString::fromStdString(profile.getUserId());
            user["email"] = "user@v2raycpp";
            user["encryption"] = "none";
            
            // Save flow (for XTLS modes)
            QString flow = QString::fromStdString(profile.getFlow());
            if (!flow.isEmpty())
            {
                user["flow"] = flow;
            }
            
            users.append(user);
            server["users"] = users;
            
            vnext.append(server);
            settings["vnext"] = vnext;
        }
        
        outbound["settings"] = settings;
        
        // Build stream settings
        QJsonObject streamSettings;
        QString network = QString::fromStdString(profile.getNetwork().empty() ? "tcp" : profile.getNetwork());
        streamSettings["network"] = network;
        
        QString security = QString::fromStdString(profile.getSecurity());
        if (!security.isEmpty() && security != "none")
        {
            streamSettings["security"] = security;
            
            if (security == "tls")
            {
                QJsonObject tlsSettings;
                QString sni = QString::fromStdString(profile.getSni());
                if (!sni.isEmpty())
                {
                    tlsSettings["serverName"] = sni;
                }
                tlsSettings["allowInsecure"] = profile.getAllowInsecure();
                
                QString alpn = QString::fromStdString(profile.getAlpn());
                if (!alpn.isEmpty())
                {
                    QJsonArray alpnArray;
                    alpnArray.append(alpn);
                    tlsSettings["alpn"] = alpnArray;
                }
                
                QString fingerprint = QString::fromStdString(profile.getFingerprint());
                if (!fingerprint.isEmpty())
                {
                    tlsSettings["fingerprint"] = fingerprint;
                }
                else
                {
                    tlsSettings["fingerprint"] = "random";
                }
                
                streamSettings["tlsSettings"] = tlsSettings;
            }
            else if (security == "reality")
            {
                QJsonObject realitySettings;
                QString sni = QString::fromStdString(profile.getSni());
                if (!sni.isEmpty())
                {
                    realitySettings["serverName"] = sni;
                }
                
                QString fingerprint = QString::fromStdString(profile.getFingerprint());
                if (!fingerprint.isEmpty())
                {
                    realitySettings["fingerprint"] = fingerprint;
                }
                
                // Save Reality-specific fields
                QString publicKey = QString::fromStdString(profile.getPublicKey());
                if (!publicKey.isEmpty())
                {
                    realitySettings["publicKey"] = publicKey;
                }
                
                QString shortId = QString::fromStdString(profile.getShortId());
                if (!shortId.isEmpty())
                {
                    realitySettings["shortId"] = shortId;
                }
                
                QString spiderX = QString::fromStdString(profile.getSpiderX());
                if (!spiderX.isEmpty())
                {
                    realitySettings["spiderX"] = spiderX;
                }
                
                streamSettings["realitySettings"] = realitySettings;
            }
        }
        
        outbound["streamSettings"] = streamSettings;
        
        // Mux settings (disabled by default)
        QJsonObject mux;
        mux["enabled"] = false;
        mux["concurrency"] = -1;
        outbound["mux"] = mux;
        
        outbounds.append(outbound);
    }
    
    // Add direct and block outbounds if not present
    bool hasDirect = false;
    bool hasBlock = false;
    for (const QJsonValue& ob : outbounds)
    {
        QString tag = ob.toObject()["tag"].toString();
        if (tag == "direct") hasDirect = true;
        if (tag == "block") hasBlock = true;
    }
    
    if (!hasDirect)
    {
        QJsonObject directOutbound;
        directOutbound["tag"] = "direct";
        directOutbound["protocol"] = "freedom";
        directOutbound["settings"] = QJsonObject();
        outbounds.append(directOutbound);
    }
    
    if (!hasBlock)
    {
        QJsonObject blockOutbound;
        blockOutbound["tag"] = "block";
        blockOutbound["protocol"] = "blackhole";
        blockOutbound["settings"] = QJsonObject();
        outbounds.append(blockOutbound);
    }
    
    root["outbounds"] = outbounds;
    
    // Ensure required top-level fields exist
    if (!root.contains("log"))
    {
        QJsonObject log;
        log["loglevel"] = "warning";
        root["log"] = log;
    }
    
    if (!root.contains("inbounds"))
    {
        QJsonArray inbounds;
        QJsonObject inbound;
        inbound["tag"] = "socks-inbound";
        inbound["protocol"] = "socks";
        inbound["listen"] = "127.0.0.1";
        inbound["port"] = AppConfig::instance().getLocalPort();
        
        QJsonObject socksSettings;
        socksSettings["auth"] = "noauth";
        socksSettings["udp"] = true;
        inbound["settings"] = socksSettings;
        
        inbounds.append(inbound);
        root["inbounds"] = inbounds;
    }
    
    if (!root.contains("routing"))
    {
        QJsonObject routing;
        routing["domainStrategy"] = "IPIfNonMatch";
        routing["mode"] = "proxy";
        
        QJsonArray rules;
        QJsonObject rule;
        rule["type"] = "field";
        rule["outboundTag"] = "direct";
        
        QJsonArray domain;
        domain.append("geosite:cn");
        rule["domain"] = domain;
        
        QJsonArray ip;
        ip.append("geoip:private");
        ip.append("geoip:cn");
        rule["ip"] = ip;
        
        rules.append(rule);
        routing["rules"] = rules;
        
        root["routing"] = routing;
    }
    
    // Write updated config
    QJsonDocument doc(root);
    QFile file(configFile);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open config.json for writing:" << configFile;
        return;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "Saved" << m_serverProfiles.size() << "servers to config.json";
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
    
    // Generate core config
    if (!generateCoreConfig(m_currentProfile))
    {
        QMessageBox::critical(this, "Error", "Failed to generate config");
        return;
    }
    
    // Get config file path
    QString configPath = AppConfig::instance().getCoreConfigPath();
    
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

    // Try to parse the URL
    ProfileItem profile;
    if (parseProfileFromUrl(text, profile))
    {
        // Add to list
        m_serverProfiles.push_back(profile);
        addServerToList(profile);

        // 选中新加入的 profile（不再调用 m_serverGrid）
        m_currentProfile = m_serverProfiles.back();
        updateStatusBar();

        // 如果使用新的 grid，可以选中最后一个项（占位）
        //if (m_serverGrid) {
            // TODO: 如果 ServerGridWidget 支持选择，调用相应方法选中最后一个项
            // e.g. m_serverGrid->selectIndex(static_cast<int>(m_serverProfiles.size()) - 1);
        //}

        // statusBar()->showMessage() removed - using statusLabel instead
    }
    else
    {
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


// onCustomContextMenu removed
void v2raycpp::onCustomContextMenu(const QPoint&) { }


// onServerDoubleClicked removed
void v2raycpp::onServerDoubleClicked() { }


// onServerSelected removed  
void v2raycpp::onServerSelected(int) { }


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

// ==================== Latency Test Functions ====================

void v2raycpp::testLatency(const QString& address, int port)
{
    QElapsedTimer timer;
    timer.start();
    
    QTcpSocket socket;
    socket.connectToHost(address, port);
    
    if (socket.waitForConnected(3000)) {
        int latency = timer.elapsed();
        qDebug() << "Latency test result:" << address << ":" << port << "-" << latency << "ms";
    } else {
        qDebug() << "Latency test failed:" << address << ":" << port;
    }
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
    if (!card || !m_geoNetwork || serverIndex < 0 || serverIndex >= static_cast<int>(m_serverProfiles.size()))
        return;

    const ProfileItem& pr = m_serverProfiles[static_cast<size_t>(serverIndex)];
    const QString host = QString::fromStdString(pr.getAddress());
    const int port = pr.getPort();
    if (host.isEmpty() || port <= 0)
        return;

    QPointer<SimpleCard> safeCard(card);
    card->setLatencyValue(kLatencyProbePending);
    card->setLocationInfo(QStringLiteral("定位中…"), QString());

    auto* watcher = new QFutureWatcher<int>(this);
    connect(watcher, &QFutureWatcher<int>::finished, this, [this, safeCard, serverIndex, host, watcher]() {
        const int latency = watcher->result();
        watcher->deleteLater();
        if (!safeCard)
            return;

        if (serverIndex >= 0 && serverIndex < static_cast<int>(m_serverProfiles.size()))
            m_serverProfiles[static_cast<size_t>(serverIndex)].setLatency(latency);
        safeCard->setLatencyValue(latency);

        const QByteArray enc = QUrl::toPercentEncoding(host);
        const QUrl url(QStringLiteral("http://ip-api.com/json/%1?fields=status,message,country,countryCode,query")
                           .arg(QString::fromUtf8(enc)));
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 (compatible; V2rayCpp/1.0)"));
        QNetworkReply* reply = m_geoNetwork->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, safeCard, serverIndex, reply]() {
            reply->deleteLater();
            if (!safeCard)
                return;

            QString countryLine;
            QString ipLine;
            if (reply->error() != QNetworkReply::NoError) {
                safeCard->setLocationInfo(QStringLiteral("未知"), QString());
                return;
            }

            const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (!doc.isObject()) {
                safeCard->setLocationInfo(QStringLiteral("未知"), QString());
                return;
            }

            const QJsonObject o = doc.object();
            if (o.value(QStringLiteral("status")).toString() != QStringLiteral("success")) {
                const QString msg = o.value(QStringLiteral("message")).toString();
                if (!msg.isEmpty())
                    safeCard->setLocationInfo(msg, QString());
                else
                    safeCard->setLocationInfo(QStringLiteral("未知"), QString());
                return;
            }

            const QString cty = o.value(QStringLiteral("country")).toString();
            const QString code = o.value(QStringLiteral("countryCode")).toString();
            const QString q = o.value(QStringLiteral("query")).toString();
            if (!cty.isEmpty()) {
                countryLine = code.isEmpty() ? cty : QStringLiteral("%1 (%2)").arg(cty, code);
            }
            ipLine = q;
            if (serverIndex >= 0 && serverIndex < static_cast<int>(m_serverProfiles.size())) {
                m_serverProfiles[static_cast<size_t>(serverIndex)].setCountry(cty.toStdString());
                m_serverProfiles[static_cast<size_t>(serverIndex)].setCountryCode(code.toStdString());
            }
            if (countryLine.isEmpty())
                countryLine = QStringLiteral("未知");
            safeCard->setLocationInfo(countryLine, ipLine);
        });
    });

    QFuture<int> fut = QtConcurrent::run([host, port]() {
        return tcpConnectLatencyMs(host, port);
    });
    watcher->setFuture(fut);
}

ProfileItem v2raycpp::getSelectedProfile() const
{
    if (m_serverProfiles.size() > 0) return m_serverProfiles[0];
    return ProfileItem();
}


bool v2raycpp::parseProfileFromUrl(const QString& url, ProfileItem& profile)
{
    // Try to parse as Trojan URL
    if (url.startsWith("trojan://"))
    {
        auto parsed = TrojanFmt::parse(url.toStdString());
        if (parsed)
        {
            profile = *parsed;
            return true;
        }
    }
    
    // Try to parse as VMess URL (simple check)
    if (url.startsWith("vmess://"))
    {
        // TODO: Implement VMess URL parsing
        QMessageBox::information(this, "Info", "VMess not supported yet");
        return false;
    }
    
    // Try to parse as VLESS URL
    if (url.startsWith("vless://"))
    {
        // TODO: Implement VLESS URL parsing
        QMessageBox::information(this, "Info", "VLESS not supported yet");
        return false;
    }
    
    return false;
}

// ==================== Traffic Statistics ====================

void v2raycpp::startStatsTimer()
{
    // Reset counters
    m_bytesReceived = 0;
    m_bytesSent = 0;
    m_lastBytesReceived = 0;
    m_lastBytesSent = 0;
    
    // Create timer if not exists
    if (!m_statsTimer)
    {
        m_statsTimer = new QTimer(this);
        connect(m_statsTimer, &QTimer::timeout, this, &v2raycpp::updateStats);
    }
    
    // Start timer with 1 second interval
    m_statsTimer->start(1000);
}

void v2raycpp::stopStatsTimer()
{
    // Stop timer
    if (m_statsTimer)
    {
        m_statsTimer->stop();
    }
    
    // Reset display
    if (ui.statSpeedDown)
    {
        ui.statSpeedDown->setText("0 KB/s");
    }
    if (ui.statSpeedUp)
    {
        ui.statSpeedUp->setText("0 KB/s");
    }

}

void v2raycpp::updateStats()
{
    // Simple implementation: generate random speed for demonstration
    // In production, you would get actual network statistics from system APIs
    
    // Simulate some traffic (replace with actual implementation)
    static int tick = 0;
    tick++;
    
    // Generate pseudo-random speed values for demo
    // Download: 100KB/s - 5MB/s
    qint64 downloadSpeed = (100 + (tick * 37) % 5000) * 1024;
    // Upload: 50KB/s - 1MB/s
    qint64 uploadSpeed = (50 + (tick * 23) % 1000) * 1024;
    
    // Format download speed
    QString downloadStr;
    if (downloadSpeed >= 1024 * 1024)
    {
        downloadStr = QString("%1 MB/s").arg(downloadSpeed / (1024.0 * 1024.0), 0, 'f', 1);
    }
    else
    {
        downloadStr = QString("%1 KB/s").arg(downloadSpeed / 1024.0, 0, 'f', 1);
    }
    
    // Format upload speed
    QString uploadStr;
    if (uploadSpeed >= 1024 * 1024)
    {
        uploadStr = QString("%1 MB/s").arg(uploadSpeed / (1024.0 * 1024.0), 0, 'f', 1);
    }
    else
    {
        uploadStr = QString("%1 KB/s").arg(uploadSpeed / 1024.0, 0, 'f', 1);
    }
    
    // Update UI labels
    if (ui.statSpeedDown)
    {
        ui.statSpeedDown->setText(downloadStr);
    }
    if (ui.statSpeedUp)
    {
        ui.statSpeedUp->setText(uploadStr);
    }

    if (ui.statUptime) {
        ui.statUptime->setText(formatFooterUptime(m_startTime, m_currentStatus));
    }

}

void v2raycpp::startReconnectTimer()
{
    if (!m_reconnectTimer) {
        m_reconnectTimer = new QTimer(this);
        connect(m_reconnectTimer, &QTimer::timeout, this, &v2raycpp::onReconnectTimeout);
    }
    // 5秒后重连
    m_reconnectTimer->start(5000);
}

void v2raycpp::stopReconnectTimer()
{
    if (m_reconnectTimer) {
        m_reconnectTimer->stop();
    }
}

void v2raycpp::onReconnectTimeout()
{
    stopReconnectTimer();
    
    if (m_currentProfile.isValid()) {
        qDebug() << "Auto reconnecting...";
        onStartClicked();
    }
}

void v2raycpp::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text);
    updateServerCardFilter();
}