#include "v2raycpp.h"
#include <QMouseEvent>
#include "TrayIcon.h"
#include "TrojanFmt.h"
#include <QFileDialog>
#include <QDateTime>
#include <QTextEdit>
#include <QPushButton>
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
#include <QElapsedTimer>
#include <QThread>

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>

#include "SimpleCard.h"

v2raycpp::v2raycpp(QWidget *parent)
    : QWidget(parent)
{
    m_dragging = false;
    m_dragPosition = QPoint();
    
    setWindowFlags(Qt::FramelessWindowHint);
    
    // Load window position from config
    {
        QFile file("E:/v2raycpp/V2ray/window_pos.json");
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                int x = obj["x"].toInt(-1);
                int y = obj["y"].toInt(-1);
                int w = obj["width"].toInt(-1);
                int h = obj["height"].toInt(-1);
                if (x >= 0 && y >= 0 && w > 0 && h > 0) {
                    setGeometry(x, y, w, h);
                }
            }
            file.close();
        }
    }
    
    ui.setupUi(this);
    
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
    
    // Initialize UI
    loadStyleSheet();
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
    QFile file("E:/v2raycpp/V2ray/window_pos.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject obj;
        obj["x"] = x();
        obj["y"] = y();
        obj["width"] = width();
        obj["height"] = height();
        QJsonDocument doc(obj);
        file.write(doc.toJson());
        file.close();
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
    // Create ServerGridWidget - serverList not in new UI
    m_serverGrid = new ServerGridWidget(this);
    m_serverGrid->setObjectName("serverGrid");
    
    // Connect grid signals
    connect(m_serverGrid, &ServerGridWidget::serverClicked, this, [this](int index) {
        if (index >= 0 && index < (int)m_serverProfiles.size()) {
            // Select and optionally connect
        }
    });
    
    connect(m_serverGrid, &ServerGridWidget::serverToggled, this, [this](int index, bool connected) {
        if (index >= 0 && index < (int)m_serverProfiles.size()) {
            if (connected) {
                m_currentProfile = m_serverProfiles[index];
                onStartClicked();
            } else {
                onStopClicked();
            }
        }
    });
}

void v2raycpp::loadStyleSheet()
{
    // Load style sheet from application directory
    QString styleFilePath = QCoreApplication::applicationDirPath() + "/style.qss";
    QFile styleFile(styleFilePath);
    if (!styleFile.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to open style.qss:" << styleFile.errorString() << "Path:" << styleFilePath;
        return;
    }
    QTextStream stream(&styleFile);
    QString styleSheet = stream.readAll();
    styleFile.close();
    if (styleSheet.isEmpty()) {
        qWarning() << "style.qss is empty!";
        return;
    }
    qWarning() << "Loaded style.qss, size:" << styleSheet.size();
    this->setStyleSheet(styleSheet);
}

void v2raycpp::initUI()
{
    // Set window title
    setWindowTitle("v2raycpp");
    
    // Set sidebar button texts to English
    if (ui.navHome) ui.navHome->setText("Home");
    if (ui.navServers) ui.navServers->setText("Servers");
    if (ui.navSettings) ui.navSettings->setText("Settings");
    
        // Server grid is created in initServerGrid()\n    // Old serverList not used\n    \n    // Connect new UI buttons
    if (ui.btnStartProxy)
    {
        connect(ui.btnStartProxy, &QPushButton::clicked, this, &v2raycpp::onStartClicked);
    }
    if (ui.toolAdd)
    {
        connect(ui.toolAdd, &QPushButton::clicked, this, &v2raycpp::onAddServerClicked);
    }
    if (ui.toolDl)
    {
        connect(ui.toolDl, &QPushButton::clicked, this, &v2raycpp::onImportClicked);
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
            ui.btnStartProxy->setText("Stop");
        }
        else
        {
            ui.btnStartProxy->setText("Start");
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
    
    // Update start time label
    if (ui.statUptime)
    {
        if (m_currentStatus == CoreStatus::Running && m_startTime.isValid())
        {
            ui.statUptime->setText("Connected: " + m_startTime.toString("HH:mm:ss"));
        }
        else
        {
            ui.statUptime->setText("Connected: --");
        }
    }
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

    // Load server list from JSON file
    QString configPath = AppConfig::instance().getConfigPath();
    QString serversFile = configPath + "/servers.json";
    
    QFile file(serversFile);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "No servers.json found, starting with empty list";
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject())
    {
        qWarning() << "Invalid servers.json format";
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray servers = root["servers"].toArray();
    
    for (int i = 0; i < servers.size(); ++i)
    {
        QJsonObject serverObj = servers[i].toObject();
        
        ProfileItem profile;
        profile.setAddress(serverObj["address"].toString().toStdString());
        profile.setPort(serverObj["port"].toInt());
        profile.setRemark(serverObj["remark"].toString().toStdString());
        profile.setPassword(serverObj["password"].toString().toStdString());
        profile.setSni(serverObj["sni"].toString().toStdString());
        profile.setNetwork(serverObj["network"].toString().toStdString());
        profile.setSecurity(serverObj["security"].toString().toStdString());
        profile.setFingerprint(serverObj["fingerprint"].toString().toStdString());
        profile.setAllowInsecure(serverObj["allowInsecure"].toBool());
        profile.setUserId(serverObj["userId"].toString().toStdString());
        profile.setAlterId(serverObj["alterId"].toString().toStdString());
        
        // Parse config type
        QString configTypeStr = serverObj["configType"].toString();
        if (configTypeStr == "VMess")
            profile.setConfigType(EConfigType::VMess);
        else if (configTypeStr == "VLESS")
            profile.setConfigType(EConfigType::VLESS);
        else if (configTypeStr == "Trojan")
            profile.setConfigType(EConfigType::Trojan);
        else if (configTypeStr == "Shadowsocks")
            profile.setConfigType(EConfigType::Shadowsocks);
        else
            profile.setConfigType(EConfigType::Trojan);
        
        if (profile.isValid())
        {
            m_serverProfiles.push_back(profile);
            addServerToList(profile);
        }
    }
    
    qDebug() << "Loaded" << m_serverProfiles.size() << "servers from config";
}

void v2raycpp::saveConfig()
{
    // Save application configuration
    AppConfig::instance().save();
    
    // Save server list to JSON file
    QString configPath = AppConfig::instance().getConfigPath();
    QString serversFile = configPath + "/servers.json";
    
    QJsonObject root;
    QJsonArray servers;
    
    for (const auto& profile : m_serverProfiles)
    {
        QJsonObject serverObj;
        serverObj["address"] = QString::fromStdString(profile.getAddress());
        serverObj["port"] = profile.getPort();
        serverObj["remark"] = QString::fromStdString(profile.getRemark());
        serverObj["password"] = QString::fromStdString(profile.getPassword());
        serverObj["sni"] = QString::fromStdString(profile.getSni());
        serverObj["network"] = QString::fromStdString(profile.getNetwork());
        serverObj["security"] = QString::fromStdString(profile.getSecurity());
        serverObj["fingerprint"] = QString::fromStdString(profile.getFingerprint());
        serverObj["allowInsecure"] = profile.getAllowInsecure();
        serverObj["userId"] = QString::fromStdString(profile.getUserId());
        serverObj["alterId"] = QString::fromStdString(profile.getAlterId());
        serverObj["configType"] = QString::fromStdString(profile.getConfigTypeString());
        
        servers.append(serverObj);
    }
    
    root["servers"] = servers;
    
    QJsonDocument doc(root);
    QFile file(serversFile);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open servers.json for writing:" << serversFile;
        return;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "Saved" << m_serverProfiles.size() << "servers to config";
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

        // Add to UI (grid)
        addServerToList(profile);

        // Set as current profile
        m_currentProfile = profile;

        // Update status bar
        updateStatusBar();

        // 如果使用新的 grid，可以选中最后一个项（占位）
        if (m_serverGrid) {
            // TODO: 如果 ServerGridWidget 支持选择，调用相应方法选中最后一个项
            // e.g. m_serverGrid->selectIndex(static_cast<int>(m_serverProfiles.size()) - 1);
        }

        // statusBar()->showMessage() removed - using statusLabel instead
    }
    else
    {
        QMessageBox::warning(this, "Warning", "Failed to parse URL");
    }
}

void v2raycpp::onAddServerClicked()
{
    // Simple dialog to add server manually
    QDialog dialog(this);
    dialog.setWindowTitle("Add Server");
    dialog.setMinimumWidth(400);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QLabel* remarkLabel = new QLabel("Remark:", &dialog);
    QLineEdit* remarkEdit = new QLineEdit(&dialog);
    layout->addWidget(remarkLabel);
    layout->addWidget(remarkEdit);
    
    QLabel* addressLabel = new QLabel("Address:", &dialog);
    QLineEdit* addressEdit = new QLineEdit(&dialog);
    layout->addWidget(addressLabel);
    layout->addWidget(addressEdit);
    
    QLabel* portLabel = new QLabel("Port:", &dialog);
    QLineEdit* portEdit = new QLineEdit(&dialog);
    layout->addWidget(portLabel);
    layout->addWidget(portEdit);
    
    QLabel* passwordLabel = new QLabel("Password:", &dialog);
    QLineEdit* passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordLabel);
    layout->addWidget(passwordEdit);
    
    QLabel* sniLabel = new QLabel("SNI (Optional):", &dialog);
    QLineEdit* sniEdit = new QLineEdit(&dialog);
    layout->addWidget(sniLabel);
    layout->addWidget(sniEdit);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        &dialog);
    layout->addWidget(buttons);
    
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted)
    {
        ProfileItem profile;
        profile.setRemark(remarkEdit->text().toStdString());
        profile.setAddress(addressEdit->text().toStdString());
        profile.setPort(portEdit->text().toInt());
        profile.setPassword(passwordEdit->text().toStdString());
        profile.setSni(sniEdit->text().toStdString());
        profile.setSecurity("tls");
        profile.setNetwork("tcp");
        profile.setConfigType(EConfigType::Trojan);
        
        if (profile.isValid())
        {
            m_serverProfiles.push_back(profile);
            addServerToList(profile);
            
            m_currentProfile = profile;
            updateStatusBar();
            //TODO:Fixit
            //ui.serverList->setCurrentRow(ui.serverList->count() - 1);
            
            // statusBar()->showMessage() removed - using statusLabel instead
        }
        else
        {
            QMessageBox::warning(this, "Warning", "Invalid server data");
        }
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
    // Test latency for all servers in the list
    for (int i = 0; i < m_serverProfiles.size(); ++i)
    {
        ProfileItem& profile = m_serverProfiles[i];

        // Test TCP connection latency
        QElapsedTimer timer;
        timer.start();

        QTcpSocket socket;
        socket.connectToHost(QString::fromStdString(profile.getAddress()), profile.getPort());

        if (socket.waitForConnected(3000)) {
            int latency = timer.elapsed();
            profile.setLatency(latency);
        }
        else {
            profile.setLatency(-1);  // Failed to connect
        }

        // Update the list item text with latency
        QString latencyStr;
        if (profile.getLatency() > 0) {
            latencyStr = QString::number(profile.getLatency()) + "ms";
        }
        else {
            latencyStr = "--";
        }

        QString itemText = QString("%1 - %2:%3 - %4")
            .arg(QString::fromStdString(profile.getRemark().empty() ?
                profile.getAddress() : profile.getRemark()))
            .arg(QString::fromStdString(profile.getAddress()))
            .arg(profile.getPort())
            .arg(latencyStr);

        // Grid update handled elsewhere
    }

    // Small delay between tests to avoid overwhelming
    QThread::msleep(100);
}

// ==================== Helper Functions ====================

void v2raycpp::addServerToList(const ProfileItem& profile)
{
    QString itemText = QString("%1 - %2:%3")
        .arg(QString::fromStdString(profile.getRemark().empty() ? 
              profile.getAddress() : profile.getRemark()))
        .arg(QString::fromStdString(profile.getAddress()))
        .arg(profile.getPort());
    
    // serverList removed - using grid only
    // Also add to new card-style grid
            // Get protocol string from config type
    QString protocol;
    if (m_serverGrid) {
        QString name = QString::fromStdString(profile.getRemark().empty() ? 
                       profile.getAddress() : profile.getRemark());
        

        switch (profile.getConfigType()) {
            case EConfigType::VMess: protocol = "VMess"; break;
            case EConfigType::VLESS: protocol = "VLESS"; break;
            case EConfigType::Trojan: protocol = "Trojan"; break;
            case EConfigType::Shadowsocks: protocol = "Shadowsocks"; break;
            case EConfigType::Socks: protocol = "Socks"; break;
            case EConfigType::Http: protocol = "HTTP"; break;
            case EConfigType::Hysteria2: protocol = "Hysteria2"; break;
            case EConfigType::Tuic: protocol = "TUIC"; break;
            default: protocol = "Unknown"; break;
        }
        
        QString flagPath = ""; // TODO: Add flag based on country
        int latency = -1; // TODO: Test latency
        bool isConnected = (m_currentStatus == CoreStatus::Running && 
                          profile.getAddress() == m_currentProfile.getAddress());
        m_serverGrid->addServer(name, latency, protocol, flagPath, isConnected);
    }
    
    // 同时将卡片添加到主 UI 的 gridLayout（若存在）
    if (ui.gridLayout)
    {
        // 标题使用备注或地址+端口
        QString label = QString::fromStdString(profile.getRemark().empty() ?
                                               profile.getAddress() : profile.getRemark());
        addCardToGrid(label, protocol);
    }
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
    QString searchText = text.trimmed().toLower();
    //TODO:Fixit
    /*for (int i = 0; i < ui.serverList->count(); ++i)
    {
        QListWidgetItem* item = ui.serverList->item(i);
        if (item)
        {
            QString itemText = item->text().toLower();
            bool match = searchText.isEmpty() || itemText.contains(searchText);
            item->setHidden(!match);
        }
    }*/
}

// 将一个卡片插入到 designer 的 gridLayout 中。
// 插入策略：如果能找到 ui.cardAddNode，则在其位置放入新卡片，并把 cardAddNode 推到下一个格子；
// 否则按当前 count 计算位置追加。
void v2raycpp::addCardToGrid(const QString &title, const QString &subText)
{
    if (!ui.gridLayout) return;

    // 创建卡片并设置样式文本
    SimpleCard *card = new SimpleCard(this);
    card->setTitle(title);
    card->setSubText(subText);
    card->setIconText("\342\236\225"); // 使用与 designer 中相同的符号作为占位图标

    QGridLayout *g = ui.gridLayout;
    const int columns = 2; // 与 ui 设计保持 2 列

    // 尝试找到 cardAddNode 在 layout 中的位置
    int addNodeIndex = -1;
    for (int i = 0; i < g->count(); ++i)
    {
        QLayoutItem *item = g->itemAt(i);
        if (!item) continue;
        QWidget *w = item->widget();
        if (w == ui.cardAddNode) { addNodeIndex = i; break; }
    }

    if (addNodeIndex != -1)
    {
        int r, c, rs, cs;
        g->getItemPosition(addNodeIndex, &r, &c, &rs, &cs);

        // 放置新卡片在 cardAddNode 的原位置
        g->addWidget(card, r, c);

        // 将 cardAddNode 移动到下一个格子
        int newR = r;
        int newC = c + 1;
        if (newC >= columns) { newC = 0; newR = r + 1; }
        g->removeWidget(ui.cardAddNode);
        g->addWidget(ui.cardAddNode, newR, newC);
    }
    else
    {
        // 退化：按当前 count 追加
        int idx = g->count();
        int r = idx / columns;
        int c = idx % columns;
        g->addWidget(card, r, c);
    }

    // 如果需要，可为 card 添加鼠标事件或点击逻辑（此处保持简单）
}





