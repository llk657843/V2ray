#include "ConfigGenerator.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

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

    // 2. Inbounds 配置（SOCKS 10808 + HTTP 10809）
    config["inbounds"] = genInbounds();

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

QJsonArray ConfigGenerator::genInbounds()
{
    QJsonArray inbounds;

    // SOCKS 入站 - 端口 10808
    QJsonObject socksInbound;
    socksInbound["tag"] = "socks-inbound";
    socksInbound["protocol"] = "socks";
    socksInbound["listen"] = "127.0.0.1";
    socksInbound["port"] = 10808;

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

    // HTTP 入站 - 端口 10809
    QJsonObject httpInbound;
    httpInbound["tag"] = "http-inbound";
    httpInbound["protocol"] = "http";
    httpInbound["listen"] = "127.0.0.1";
    httpInbound["port"] = 10809;

    QJsonObject httpSettings;
    httpSettings["auth"] = "noauth";
    httpSettings["udp"] = true;
    httpSettings["ip"] = "127.0.0.1";
    httpInbound["settings"] = httpSettings;

    inbounds.append(httpInbound);

    return inbounds;
}

QJsonArray ConfigGenerator::genOutbounds(const ProfileItem& node)
{
    QJsonArray outbounds;

    // Trojan 出站（代理）
    QJsonObject proxyOutbound;
    proxyOutbound["tag"] = "proxy";
    proxyOutbound["protocol"] = "trojan";

    // Trojan 服务器配置
    QJsonObject trojanSettings;
    QJsonArray servers;

    QJsonObject server;
    server["address"] = QString::fromStdString(node.getAddress());
    server["port"] = node.getPort();
    server["password"] = QString::fromStdString(node.getPassword());

    // Flow 设置
    if (!node.getFlow().empty())
    {
        server["flow"] = QString::fromStdString(node.getFlow());
    }

    server["level"] = 1;
    servers.append(server);

    trojanSettings["servers"] = servers;
    proxyOutbound["settings"] = trojanSettings;

    // Stream Settings - TCP + TLS
    QJsonObject streamSettings;
    streamSettings["network"] = QString::fromStdString(node.getNetwork().empty() ? "tcp" : node.getNetwork());
    streamSettings["security"] = QString::fromStdString(node.getSecurity().empty() ? "tls" : node.getSecurity());

    // TLS 设置
    QJsonObject tlsSettings;
    if (!node.getSni().empty())
    {
        tlsSettings["serverName"] = QString::fromStdString(node.getSni());
    }
    else
    {
        tlsSettings["serverName"] = QString::fromStdString(node.getAddress());
    }

    if (!node.getFingerprint().empty())
    {
        tlsSettings["fingerprint"] = QString::fromStdString(node.getFingerprint());
    }

    if (!node.getAlpn().empty())
    {
        QString alpnStr = QString::fromStdString(node.getAlpn());
        tlsSettings["alpn"] = QJsonArray::fromStringList(alpnStr.split(","));
    }

    tlsSettings["allowInsecure"] = node.getAllowInsecure();

    streamSettings["tlsSettings"] = tlsSettings;
    proxyOutbound["streamSettings"] = streamSettings;

    outbounds.append(proxyOutbound);

    // Direct 出站（直连）
    QJsonObject directOutbound;
    directOutbound["tag"] = "direct";
    directOutbound["protocol"] = "freedom";

    QJsonObject directSettings;
    directSettings["domainStrategy"] = "UseIP";
    directOutbound["settings"] = directSettings;

    outbounds.append(directOutbound);

    // Block 出站（拦截）
    QJsonObject blockOutbound;
    blockOutbound["tag"] = "block";
    blockOutbound["protocol"] = "blackhole";

    QJsonObject blockSettings;
    blockSettings["response"] = QJsonObject();
    blockOutbound["settings"] = blockSettings;

    outbounds.append(blockOutbound);

    // DNS 出站
    QJsonObject dnsOutbound;
    dnsOutbound["tag"] = "dns-outbound";
    dnsOutbound["protocol"] = "dns";

    outbounds.append(dnsOutbound);

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