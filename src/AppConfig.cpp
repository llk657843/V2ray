#include "AppConfig.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>

// ============ CoreBasicItem 实现 ============

QJsonObject CoreBasicItem::toJson() const
{
    QJsonObject json;
    json["logEnabled"] = logEnabled;
    json["logLevel"] = QString::fromStdString(logLevel);
    json["muxEnabled"] = muxEnabled;
    json["defAllowInsecure"] = defAllowInsecure;
    json["defFingerprint"] = QString::fromStdString(defFingerprint);
    json["defUserAgent"] = QString::fromStdString(defUserAgent);
    return json;
}

void CoreBasicItem::fromJson(const QJsonObject& json)
{
    logEnabled = json["logEnabled"].toBool(true);
    logLevel = json["logLevel"].toString("warning").toStdString();
    muxEnabled = json["muxEnabled"].toBool(false);
    defAllowInsecure = json["defAllowInsecure"].toBool(false);
    defFingerprint = json["defFingerprint"].toString("").toStdString();
    defUserAgent = json["defUserAgent"].toString("").toStdString();
}

// ============ InboundItem 实现 ============

QJsonObject InboundItem::toJson() const
{
    QJsonObject json;
    json["localPort"] = localPort;
    json["protocol"] = QString::fromStdString(protocol);
    json["udpEnabled"] = udpEnabled;
    json["sniffingEnabled"] = sniffingEnabled;
    
    QJsonArray destOverrideArray;
    for (const auto& dest : destOverride)
    {
        destOverrideArray.append(QString::fromStdString(dest));
    }
    json["destOverride"] = destOverrideArray;
    
    json["routeOnly"] = routeOnly;
    json["allowLANConn"] = allowLANConn;
    json["user"] = QString::fromStdString(user);
    json["pass"] = QString::fromStdString(pass);
    return json;
}

void InboundItem::fromJson(const QJsonObject& json)
{
    localPort = json["localPort"].toInt(10808);
    protocol = json["protocol"].toString("socks").toStdString();
    udpEnabled = json["udpEnabled"].toBool(true);
    sniffingEnabled = json["sniffingEnabled"].toBool(true);
    
    destOverride.clear();
    QJsonArray destOverrideArray = json["destOverride"].toArray();
    for (const auto& dest : destOverrideArray)
    {
        destOverride.push_back(dest.toString().toStdString());
    }
    if (destOverride.empty())
    {
        destOverride = {"http", "tls"};
    }
    
    routeOnly = json["routeOnly"].toBool(false);
    allowLANConn = json["allowLANConn"].toBool(false);
    user = json["user"].toString("").toStdString();
    pass = json["pass"].toString("").toStdString();
}

// ============ RoutingItem 实现 ============

QJsonObject RoutingItem::toJson() const
{
    QJsonObject json;
    json["mode"] = QString::fromStdString(mode);
    json["domainStrategy"] = QString::fromStdString(domainStrategy);
    return json;
}

void RoutingItem::fromJson(const QJsonObject& json)
{
    mode = json["mode"].toString("proxy").toStdString();
    domainStrategy = json["domainStrategy"].toString("IPIfNonMatch").toStdString();
}

// ============ GUIItem 实现 ============

QJsonObject GUIItem::toJson() const
{
    QJsonObject json;
    json["autoRun"] = autoRun;
    json["enableStatistics"] = enableStatistics;
    json["displayRealTimeSpeed"] = displayRealTimeSpeed;
    return json;
}

void GUIItem::fromJson(const QJsonObject& json)
{
    autoRun = json["autoRun"].toBool(false);
    enableStatistics = json["enableStatistics"].toBool(false);
    displayRealTimeSpeed = json["displayRealTimeSpeed"].toBool(false);
}

// ============ SystemProxyItem 实现 ============

QJsonObject SystemProxyItem::toJson() const
{
    QJsonObject json;
    json["systemProxyEnabled"] = systemProxyEnabled;
    json["proxyMode"] = static_cast<int>(proxyMode);
    json["systemProxyExceptions"] = QString::fromStdString(systemProxyExceptions);
    json["notProxyLocalAddress"] = notProxyLocalAddress;
    return json;
}

void SystemProxyItem::fromJson(const QJsonObject& json)
{
    systemProxyEnabled = json["systemProxyEnabled"].toBool(false);
    proxyMode = static_cast<ProxyMode>(json["proxyMode"].toInt(1));
    systemProxyExceptions = json["systemProxyExceptions"].toString("").toStdString();
    notProxyLocalAddress = json["notProxyLocalAddress"].toBool(true);
}

// ============ AppConfig 实现 ============

QString AppConfig::getConfigPath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath + "/v2raycpp";
}

QString AppConfig::getConfigFilePath() const
{
    return getConfigPath() + "/config.json";
}

QString AppConfig::getDefaultCorePath() const
{
    // Get application directory
    QString appPath = QCoreApplication::applicationDirPath();
    
#ifdef Q_OS_WIN
    return appPath + "/xray.exe";
#else
    return appPath + "/xray";
#endif
}

QString AppConfig::getDefaultConfigPath() const
{
    // Get config directory in AppData
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    
    // Ensure directory exists
    QDir dir(configPath);
    if (!dir.exists())
    {
        dir.mkpath(configPath);
    }
    
    return configPath + "/config.json";
}

bool AppConfig::load()
{
    QString configFile = getConfigFilePath();
    QFile file(configFile);
    
    if (!file.exists())
    {
        qDebug() << "Config file does not exist, using default config:" << configFile;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open config file:" << configFile;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "Failed to parse config file:" << parseError.errorString();
        return false;
    }
    
    if (!doc.isObject())
    {
        qWarning() << "Config file is not a valid JSON object";
        return false;
    }
    
    QJsonObject json = doc.object();
    
    // 加载各个配置项
    if (json.contains("coreBasic"))
    {
        m_coreBasic.fromJson(json["coreBasic"].toObject());
    }
    
    if (json.contains("inbound"))
    {
        m_inbound.fromJson(json["inbound"].toObject());
    }
    
    if (json.contains("routing"))
    {
        m_routing.fromJson(json["routing"].toObject());
    }
    
    if (json.contains("gui"))
    {
        m_gui.fromJson(json["gui"].toObject());
    }
    
    if (json.contains("systemProxy"))
    {
        m_systemProxy.fromJson(json["systemProxy"].toObject());
    }
    
    qDebug() << "Config loaded from:" << configFile;
    return true;
}

bool AppConfig::save()
{
    QString configPath = getConfigPath();
    
    // 确保配置目录存在
    QDir dir(configPath);
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            qWarning() << "Failed to create config directory:" << configPath;
            return false;
        }
    }
    
    QString configFile = getConfigFilePath();
    QFile file(configFile);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qWarning() << "Failed to open config file for writing:" << configFile;
        return false;
    }
    
    // 构建 JSON 对象
    QJsonObject json;
    json["coreBasic"] = m_coreBasic.toJson();
    json["inbound"] = m_inbound.toJson();
    json["routing"] = m_routing.toJson();
    json["gui"] = m_gui.toJson();
    json["systemProxy"] = m_systemProxy.toJson();
    
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Indented);
    
    file.write(data);
    file.close();
    
    qDebug() << "Config saved to:" << configFile;
    return true;
}

void AppConfig::resetToDefault()
{
    m_coreBasic = CoreBasicItem();
    m_inbound = InboundItem();
    m_routing = RoutingItem();
    m_gui = GUIItem();
    m_systemProxy = SystemProxyItem();
}
