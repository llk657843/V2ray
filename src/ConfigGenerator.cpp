#include "ConfigGenerator.h"
#include "AppConfig.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QDebug>

QString ConfigGenerator::generateClientConfig(const ProfileItem& node, const QString& routingMode)
{
    ERoutingMode mode = Routing::getModeFromString(routingMode.toStdString());
    return generateClientConfig(node, mode);
}

QString ConfigGenerator::generateClientConfig(const ProfileItem& node, ERoutingMode mode)
{
    // 构建完整的 V2Ray 配置
    QJsonObject config;

    // 1. Log 配置
    QJsonObject logObj;
    logObj["loglevel"] = "warning";
    config["log"] = logObj;

    // 2. Inbounds 配置（SOCKS 10808 + HTTP 10809 + 端口转发）
    config["inbounds"] = genInbounds(node);

    // 3. Outbounds 配置（Trojan 出站）
    config["outbounds"] = genOutbounds(node);

    // 4. Routing 配置
    config["routing"] = genRouting(mode);

    // 5. DNS 配置
    config["dns"] = genDns();

    // 转换为 JSON 字符串
    QJsonDocument doc(config);
    return doc.toJson(QJsonDocument::Indented);
}

bool ConfigGenerator::generateAndWriteConfig(const ProfileItem& node, const QString& filePath)
{
    QString configJson = generateClientConfig(node, ERoutingMode::Proxy);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open config file for writing:" << filePath;
        return false;
    }
    
    file.write(configJson.toUtf8());
    file.close();
    
    return true;
}

QJsonArray ConfigGenerator::genInbounds(const ProfileItem& node)
{
    QJsonArray inbounds;

    // SOCKS 入站
    QJsonObject socksInbound;
    socksInbound["tag"] = "socks-inbound";
    socksInbound["protocol"] = "socks";
    socksInbound["listen"] = "127.0.0.1";
    socksInbound["port"] = AppConfig::instance().getLocalPort();

    QJsonObject socksSettings;
    socksSettings["auth"] = "noauth";
    socksSettings["udp"] = true;
    socksSettings["ip"] = "127.0.0.1";
    socksInbound["settings"] = socksSettings;

    // SOCKS sniffing 配置
    QJsonObject sniffing;
    sniffing["enabled"] = true;
    sniffing["destOverride"] = QJsonArray::fromStringList({"http", "tls"});
    socksInbound["sniffing"] = sniffing;

    inbounds.append(socksInbound);

    // HTTP 入站
    QJsonObject httpInbound;
    httpInbound["tag"] = "http-inbound";
    httpInbound["protocol"] = "http";
    httpInbound["listen"] = "127.0.0.1";
    httpInbound["port"] = AppConfig::instance().getLocalPort() + 1; // 默认 +1

    QJsonObject httpSettings;
    httpSettings["auth"] = "noauth";
    httpSettings["udp"] = true;
    httpSettings["ip"] = "127.0.0.1";
    httpInbound["settings"] = httpSettings;

    inbounds.append(httpInbound);

    // 端口转发配置
    if (node.getLocalPort() > 0 && !node.getForwardAddress().empty() && node.getForwardPort() > 0)
    {
        QJsonObject portForwardInbound;
        portForwardInbound["tag"] = "port-forward-inbound";
        portForwardInbound["protocol"] = "dokodemo-door";
        portForwardInbound["listen"] = "127.0.0.1";
        portForwardInbound["port"] = node.getLocalPort();

        QJsonObject portForwardSettings;
        portForwardSettings["address"] = QString::fromStdString(node.getForwardAddress());
        portForwardSettings["port"] = node.getForwardPort();
        portForwardSettings["network"] = "tcp,udp";
        portForwardInbound["settings"] = portForwardSettings;

        inbounds.append(portForwardInbound);
    }

    return inbounds;
}

QJsonArray ConfigGenerator::genOutbounds(const ProfileItem& node)
{
    QJsonArray outbounds;

    // Determine protocol based on profile
    QString protocol;
    switch (node.getConfigType())
    {
        case EConfigType::VMess: protocol = "vmess"; break;
        case EConfigType::VLESS: protocol = "vless"; break;
        case EConfigType::Trojan: protocol = "trojan"; break;
        case EConfigType::Shadowsocks: protocol = "shadowsocks"; break;
        default: protocol = "trojan";
    }

    // Proxy outbound
    QJsonObject proxyOutbound;
    proxyOutbound["tag"] = "proxy";
    proxyOutbound["protocol"] = protocol;

    // Build settings based on protocol
    QJsonObject settings;
    if (protocol == "trojan")
    {
        QJsonArray servers;
        QJsonObject server;
        server["address"] = QString::fromStdString(node.getAddress());
        server["port"] = node.getPort();
        server["password"] = QString::fromStdString(node.getPassword());
        if (!node.getFlow().empty()) server["flow"] = QString::fromStdString(node.getFlow());
        server["level"] = 1;
        servers.append(server);
        settings["servers"] = servers;
    }
    else if (protocol == "vmess")
    {
        QJsonArray vnext;
        QJsonObject server;
        server["address"] = QString::fromStdString(node.getAddress());
        server["port"] = node.getPort();
        
        QJsonArray users;
        QJsonObject user;
        user["id"] = QString::fromStdString(node.getUserId());
        user["alterId"] = QString::fromStdString(node.getAlterId()).toInt();
        user["security"] = "auto";
        user["level"] = 1;
        users.append(user);
        server["users"] = users;
        vnext.append(server);
        settings["vnext"] = vnext;
    }
    // TODO: Add VLESS and Shadowsocks
    
    proxyOutbound["settings"] = settings;

    // Stream Settings
    QJsonObject streamSettings;
    streamSettings["network"] = QString::fromStdString(node.getNetwork().empty() ? "tcp" : node.getNetwork());
    streamSettings["security"] = QString::fromStdString(node.getSecurity().empty() ? "none" : node.getSecurity());

    if (streamSettings["security"] == "tls")
    {
        QJsonObject tlsSettings;
        tlsSettings["serverName"] = QString::fromStdString(node.getSni().empty() ? node.getAddress() : node.getSni());
        tlsSettings["allowInsecure"] = node.getAllowInsecure();
        if (!node.getFingerprint().empty()) tlsSettings["fingerprint"] = QString::fromStdString(node.getFingerprint());
        if (!node.getAlpn().empty()) tlsSettings["alpn"] = QJsonArray::fromStringList(QString::fromStdString(node.getAlpn()).split(","));
        streamSettings["tlsSettings"] = tlsSettings;
    }

    proxyOutbound["streamSettings"] = streamSettings;
    outbounds.append(proxyOutbound);

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

    return outbounds;
}

QJsonObject ConfigGenerator::genRouting(ERoutingMode mode)
{
    return Routing::genRouting(mode);
}

QJsonObject ConfigGenerator::genDns()
{
    QJsonObject dns;

    // DNS 服务器列表
    QJsonArray servers;

    // Google DNS
    QJsonObject googleDns;
    googleDns["address"] = "8.8.8.8";
    googleDns["port"] = 53;
    servers.append(googleDns);

    // Cloudflare DNS
    QJsonObject cloudflareDns;
    cloudflareDns["address"] = "1.1.1.1";
    cloudflareDns["port"] = 53;
    servers.append(cloudflareDns);

    // OpenDNS
    QJsonObject openDns;
    openDns["address"] = "208.67.222.222";
    openDns["port"] = 53;
    servers.append(openDns);

    // 国内 DNS
    QJsonObject cnDns;
    cnDns["address"] = "114.114.114.114";
    cnDns["port"] = 53;
    cnDns["domains"] = QJsonArray::fromStringList({"geosite:cn", "geosite:geolocation-cn"});
    servers.append(cnDns);

    dns["servers"] = servers;

    // DNS 主机配置
    QJsonObject hosts;

    // 本地hosts
    hosts["localhost"] = QJsonArray::fromStringList({"127.0.0.1"});

    dns["hosts"] = hosts;

    // DNS 标签
    dns["tag"] = "dns-inbound";

    return dns;
}
