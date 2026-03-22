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

AppConfig::AppConfig()
{
    m_corePath = getDefaultCorePath();
    m_coreConfigPath = getDefaultConfigPath();
}

QString AppConfig::getConfigPath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (!dir.exists()) dir.mkpath(".");
    return appDataPath;
}

QString AppConfig::getDefaultCorePath() const
{
    QString appPath = QCoreApplication::applicationDirPath();
#ifdef Q_OS_WIN
    return appPath + "/xray.exe";
#else
    return appPath + "/xray";
#endif
}

QString AppConfig::getDefaultConfigPath() const
{
    return getConfigPath() + "/config.json";
}

void AppConfig::load()
{
    QString configFile = getDefaultConfigPath();
    QFile file(configFile);
    
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "No config.json found, using defaults";
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull() || !doc.isObject()) return;
    
    QJsonObject json = doc.object();
    m_coreBasic.fromJson(json["coreBasic"].toObject());
    m_inbound.fromJson(json["inbound"].toObject());
    m_routing.fromJson(json["routing"].toObject());
    m_gui.fromJson(json["gui"].toObject());
    m_systemProxy.fromJson(json["systemProxy"].toObject());
    
    if (json.contains("corePath")) m_corePath = json["corePath"].toString();
    if (json.contains("coreConfigPath")) m_coreConfigPath = json["coreConfigPath"].toString();

    loadServerProfiles();
}

void AppConfig::save()
{
    QString configDir = getConfigPath();
    QDir().mkpath(configDir);
    
    QFile file(getDefaultConfigPath());
    if (!file.open(QIODevice::WriteOnly)) return;
    
    QJsonObject json;
    json["coreBasic"] = m_coreBasic.toJson();
    json["inbound"] = m_inbound.toJson();
    json["routing"] = m_routing.toJson();
    json["gui"] = m_gui.toJson();
    json["systemProxy"] = m_systemProxy.toJson();
    json["corePath"] = m_corePath;
    json["coreConfigPath"] = m_coreConfigPath;
    
    file.write(QJsonDocument(json).toJson());
    file.close();

    saveServerProfiles();
}

void AppConfig::loadServerProfiles()
{
    QString serversFile = getConfigPath() + "/servers.json";
    QFile file(serversFile);
    if (!file.open(QIODevice::ReadOnly)) return;
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull() || !doc.isObject()) return;
    
    m_serverProfiles.clear();
    QJsonArray servers = doc.object()["servers"].toArray();
    for (const auto& serverVal : servers)
    {
        QJsonObject obj = serverVal.toObject();
        ProfileItem profile;
        profile.setAddress(obj["address"].toString().toStdString());
        profile.setPort(obj["port"].toInt());
        profile.setRemark(obj["remark"].toString().toStdString());
        profile.setPassword(obj["password"].toString().toStdString());
        profile.setSni(obj["sni"].toString().toStdString());
        profile.setNetwork(obj["network"].toString().toStdString());
        profile.setSecurity(obj["security"].toString().toStdString());
        profile.setFingerprint(obj["fingerprint"].toString().toStdString());
        profile.setAllowInsecure(obj["allowInsecure"].toBool());
        profile.setUserId(obj["userId"].toString().toStdString());
        profile.setAlterId(obj["alterId"].toString().toStdString());
        
        QString type = obj["configType"].toString();
        if (type == "VMess") profile.setConfigType(EConfigType::VMess);
        else if (type == "VLESS") profile.setConfigType(EConfigType::VLESS);
        else if (type == "Trojan") profile.setConfigType(EConfigType::Trojan);
        else if (type == "Shadowsocks") profile.setConfigType(EConfigType::Shadowsocks);
        
        if (profile.isValid()) m_serverProfiles.push_back(profile);
    }
}

void AppConfig::saveServerProfiles()
{
    QString serversFile = getConfigPath() + "/servers.json";
    QFile file(serversFile);
    if (!file.open(QIODevice::WriteOnly)) return;
    
    QJsonArray servers;
    for (const auto& profile : m_serverProfiles)
    {
        QJsonObject obj;
        obj["address"] = QString::fromStdString(profile.getAddress());
        obj["port"] = profile.getPort();
        obj["remark"] = QString::fromStdString(profile.getRemark());
        obj["password"] = QString::fromStdString(profile.getPassword());
        obj["sni"] = QString::fromStdString(profile.getSni());
        obj["network"] = QString::fromStdString(profile.getNetwork());
        obj["security"] = QString::fromStdString(profile.getSecurity());
        obj["fingerprint"] = QString::fromStdString(profile.getFingerprint());
        obj["allowInsecure"] = profile.getAllowInsecure();
        obj["userId"] = QString::fromStdString(profile.getUserId());
        obj["alterId"] = QString::fromStdString(profile.getAlterId());
        obj["configType"] = QString::fromStdString(profile.getConfigTypeString());
        servers.append(obj);
    }
    
    QJsonObject root;
    root["servers"] = servers;
    file.write(QJsonDocument(root).toJson());
    file.close();
}

void AppConfig::resetToDefault()
{
    m_coreBasic = CoreBasicItem();
    m_inbound = InboundItem();
    m_routing = RoutingItem();
    m_gui = GUIItem();
    m_systemProxy = SystemProxyItem();
}
