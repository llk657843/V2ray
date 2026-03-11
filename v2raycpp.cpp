#include "v2raycpp.h"
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
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

v2raycpp::v2raycpp(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    
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

void v2raycpp::loadStyleSheet()
{
    QFile styleFile("style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        this->setStyleSheet(styleSheet);
        styleFile.close();
    }
}

void v2raycpp::initUI()
{
    // Set window title
    setWindowTitle("v2raycpp");
    
    // Connect double click signal for server list
    connect(ui.serverList, &QListWidget::doubleClicked, 
            this, &v2raycpp::onServerDoubleClicked);
    
    // Connect toolbar actions
    if (ui.actionStart)
    {
        connect(ui.actionStart, &QAction::triggered, this, &v2raycpp::onStartClicked);
    }
    if (ui.actionStop)
    {
        connect(ui.actionStop, &QAction::triggered, this, &v2raycpp::onStopClicked);
    }
    if (ui.actionImport)
    {
        connect(ui.actionImport, &QAction::triggered, this, &v2raycpp::onImportClicked);
    }
    if (ui.actionAdd)
    {
        connect(ui.actionAdd, &QAction::triggered, this, &v2raycpp::onAddServerClicked);
    }
    if (ui.actionSettings)
    {
        connect(ui.actionSettings, &QAction::triggered, this, &v2raycpp::onSettingsClicked);
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
    if (ui.statusLabel)
    {
        ui.statusLabel->setText(statusText);
    }
    
    // Update toolbar actions enabled state
    if (ui.actionStart)
    {
        ui.actionStart->setEnabled(m_currentStatus != CoreStatus::Running);
    }
    if (ui.actionStop)
    {
        ui.actionStop->setEnabled(m_currentStatus == CoreStatus::Running);
    }
}

void v2raycpp::updateStatusBar()
{
    // Update current node label
    if (ui.currentNodeLabel)
    {
        if (m_currentProfile.isValid())
        {
            QString nodeInfo = QString("Server: %1 - %2:%3")
                .arg(QString::fromStdString(m_currentProfile.getRemark().empty() ? 
                      m_currentProfile.getAddress() : m_currentProfile.getRemark()))
                .arg(QString::fromStdString(m_currentProfile.getAddress()))
                .arg(m_currentProfile.getPort());
            ui.currentNodeLabel->setText(nodeInfo);
        }
        else
        {
            ui.currentNodeLabel->setText("No Server");
        }
    }
    
    // Update start time label
    if (ui.startTimeLabel)
    {
        if (m_currentStatus == CoreStatus::Running && m_startTime.isValid())
        {
            ui.startTimeLabel->setText("Connected: " + m_startTime.toString("HH:mm:ss"));
        }
        else
        {
            ui.startTimeLabel->setText("Connected: --");
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
    
    QJsonObject listenObj;
    listenObj["address"] = "127.0.0.1";
    listenObj["port"] = AppConfig::instance().getLocalPort();
    inbound["listen"] = listenObj;
    
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
        QJsonObject trojanSettings;
        trojanSettings["address"] = profile.getAddress().c_str();
        trojanSettings["password"] = profile.getPassword().c_str();
        
        // TLS settings
        if (profile.getSecurity() == "tls")
        {
            QJsonObject tlsSettings;
            tlsSettings["serverName"] = profile.getSni().c_str();
            if (!profile.getFingerprint().empty())
            {
                tlsSettings["fingerprint"] = profile.getFingerprint().c_str();
            }
            tlsSettings["allowInsecure"] = profile.getAllowInsecure();
            
            QJsonObject streamSettings;
            streamSettings["network"] = profile.getNetwork().c_str();
            streamSettings["security"] = "tls";
            streamSettings["tlsSettings"] = tlsSettings;
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
}

void v2raycpp::saveConfig()
{
    // Save application configuration
    AppConfig::instance().save();
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
        int currentRow = ui.serverList->currentRow();
        if (currentRow >= 0 && currentRow < (int)m_serverProfiles.size())
        {
            m_currentProfile = m_serverProfiles[currentRow];
        }
        else
        {
            QMessageBox::warning(this, "Warning", "Please select a server");
            return;
        }
    }
    
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
        
        // Update UI
        updateStatusBar();
        
        statusBar()->showMessage("Started", 3000);
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
    
    // Stop core
    if (m_coreManager->stopCore())
    {
        // Clear system proxy
        m_sysProxyHandler->clearProxy();
        
        // Reset start time
        m_startTime = QDateTime();
        
        // Update UI
        updateStatusBar();
        
        statusBar()->showMessage("Stopped", 3000);
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
        
        // Add to UI
        addServerToList(profile);
        
        // Set as current profile
        m_currentProfile = profile;
        
        // Update status bar
        updateStatusBar();
        
        // Select the new item
        ui.serverList->setCurrentRow(ui.serverList->count() - 1);
        
        statusBar()->showMessage("Server imported", 3000);
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
            
            ui.serverList->setCurrentRow(ui.serverList->count() - 1);
            
            statusBar()->showMessage("Server added", 3000);
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

void v2raycpp::onServerDoubleClicked(const QModelIndex& index)
{
    int currentRow = index.row();
    if (currentRow >= 0 && currentRow < (int)m_serverProfiles.size())
    {
        // Set as current profile
        m_currentProfile = m_serverProfiles[currentRow];
        
        // Update status bar
        updateStatusBar();
        
        // Start proxy
        onStartClicked();
    }
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
    statusBar()->showMessage("Error: " + error, 5000);
}

void v2raycpp::onStatusChanged(CoreStatus status)
{
    m_currentStatus = status;
    updateUIStatus();
    
    // Handle specific status changes
    switch (status)
    {
        case CoreStatus::Running:
            statusBar()->showMessage("Running", 3000);
            break;
        case CoreStatus::Stopped:
            // Clear proxy when stopped
            m_sysProxyHandler->clearProxy();
            statusBar()->showMessage("Stopped", 3000);
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
        statusBar()->showMessage("System proxy enabled", 3000);
    }
    else
    {
        QMessageBox::warning(this, "Warning", "Please start proxy first");
    }
}

void v2raycpp::onDisableSystemProxy()
{
    m_sysProxyHandler->clearProxy();
    statusBar()->showMessage("System proxy disabled", 3000);
}

// ==================== Helper Functions ====================

void v2raycpp::addServerToList(const ProfileItem& profile)
{
    QString itemText = QString("%1 - %2:%3")
        .arg(QString::fromStdString(profile.getRemark().empty() ? 
              profile.getAddress() : profile.getRemark()))
        .arg(QString::fromStdString(profile.getAddress()))
        .arg(profile.getPort());
    
    ui.serverList->addItem(itemText);
}

ProfileItem v2raycpp::getSelectedProfile() const
{
    int currentRow = ui.serverList->currentRow();
    if (currentRow >= 0 && currentRow < (int)m_serverProfiles.size())
    {
        return m_serverProfiles[currentRow];
    }
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
