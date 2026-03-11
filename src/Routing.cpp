#include "Routing.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

QJsonObject Routing::genRouting(ERoutingMode mode)
{
    switch (mode)
    {
    case ERoutingMode::Proxy:
    case ERoutingMode::Global:
        return genRoutingProxy();
    case ERoutingMode::Direct:
        return genRoutingDirect();
    case ERoutingMode::Rule:
    default:
        return genRoutingRule();
    }
}

QJsonObject Routing::genRoutingProxy()
{
    QJsonObject routing;

    // 域名解析策略
    routing["domainStrategy"] = "AsIs";

    // 路由规则 - 所有流量走代理
    QJsonArray rules;

    // 默认规则：所有流量 -> proxy
    QJsonObject defaultRule;
    defaultRule["type"] = "field";
    defaultRule["outboundTag"] = "proxy";
    defaultRule["network"] = "tcp,udp";
    rules.append(defaultRule);

    routing["rules"] = rules;

    return routing;
}

QJsonObject Routing::genRoutingDirect()
{
    QJsonObject routing;

    // 域名解析策略
    routing["domainStrategy"] = "AsIs";

    // 路由规则 - 所有流量直连
    QJsonArray rules;

    // 默认规则：所有流量 -> direct
    QJsonObject defaultRule;
    defaultRule["type"] = "field";
    defaultRule["outboundTag"] = "direct";
    defaultRule["network"] = "tcp,udp";
    rules.append(defaultRule);

    routing["rules"] = rules;

    return routing;
}

QJsonObject Routing::genRoutingRule()
{
    QJsonObject routing;

    // 域名解析策略
    routing["domainStrategy"] = "IPIfNonMatch";

    // 路由规则
    QJsonArray rules;

    // ========== 1. 域名规则 ==========
    
    // 规则1：广告域名 -> block
    QJsonObject adRule;
    adRule["type"] = "field";
    adRule["outboundTag"] = "block";
    QJsonArray adDomains;
    adDomains.append("geosite:category-ads-all");
    adRule["domain"] = adDomains;
    rules.append(adRule);

    // 规则2：国内域名 -> direct
    QJsonObject cnRule;
    cnRule["type"] = "field";
    cnRule["outboundTag"] = "direct";
    QJsonArray cnDomains;
    cnDomains.append("geosite:cn");
    cnDomains.append("geosite:geolocation-cn");
    cnRule["domain"] = cnDomains;
    rules.append(cnRule);

    // 规则3：常见国内网站域名 -> direct（简化版）
    QJsonObject cnSitesRule;
    cnSitesRule["type"] = "field";
    cnSitesRule["outboundTag"] = "direct";
    QJsonArray cnSitesDomains;
    cnSitesDomains.append("aqicn.org");
    cnSitesDomains.append("baidu.com");
    cnSitesDomains.append("bilibili.com");
    cnSitesDomains.append("cn");
    cnSitesDomains.append("qq.com");
    cnSitesDomains.append("taobao.com");
    cnSitesDomains.append("tmall.com");
    cnSitesDomains.append("jd.com");
    cnSitesDomains.append("163.com");
    cnSitesDomains.append("126.com");
    cnSitesDomains.append("sohu.com");
    cnSitesDomains.append("sina.com.cn");
    cnSitesDomains.append("ifeng.com");
    cnSitesDomains.append("youku.com");
    cnSitesDomains.append("iQiyi.com");
    cnSitesDomains.append("douyin.com");
    cnSitesDomains.append("zhihu.com");
    cnSitesDomains.append("csdn.net");
    cnSitesDomains.append("gitee.com");
    cnSitesDomains.append("cnblogs.com");
    cnSitesRule["domain"] = cnSitesDomains;
    rules.append(cnSitesRule);

    // ========== 2. IP 规则 ==========

    // 规则4：局域网 IP -> direct
    QJsonObject lanRule;
    lanRule["type"] = "field";
    lanRule["outboundTag"] = "direct";
    QJsonArray lanIps;
    lanIps.append("geoip:private");
    lanRule["ip"] = lanIps;
    rules.append(lanRule);

    // 规则5：国内 IP -> direct
    QJsonObject cnIpRule;
    cnIpRule["type"] = "field";
    cnIpRule["outboundTag"] = "direct";
    QJsonArray cnIps;
    cnIps.append("geoip:cn");
    cnIpRule["ip"] = cnIps;
    rules.append(cnIpRule);

    // 规则6：简化版国内 IP 段（使用预定义列表）
    QJsonObject cnIpRangeRule;
    cnIpRangeRule["type"] = "field";
    cnIpRangeRule["outboundTag"] = "direct";
    QJsonArray cnIpRanges = getCnIps();
    cnIpRangeRule["ip"] = cnIpRanges;
    rules.append(cnIpRangeRule);

    // ========== 3. 端口规则 ==========

    // 规则7：常用端口 -> proxy（80, 443）
    // 注：根据任务要求，常用端口也走代理，这里与默认规则合并

    // ========== 4. 默认规则 ==========

    // 规则8：默认 -> proxy
    QJsonObject defaultRule;
    defaultRule["type"] = "field";
    defaultRule["outboundTag"] = "proxy";
    defaultRule["network"] = "tcp,udp";
    rules.append(defaultRule);

    routing["rules"] = rules;

    return routing;
}

ERoutingMode Routing::getModeFromString(const QString& modeStr)
{
    QString lowerMode = modeStr.toLower();
    if (lowerMode == "proxy" || lowerMode == "global")
    {
        return ERoutingMode::Proxy;
    }
    else if (lowerMode == "direct")
    {
        return ERoutingMode::Direct;
    }
    else  // default "rule"
    {
        return ERoutingMode::Rule;
    }
}

ERoutingMode Routing::getModeFromString(const std::string& modeStr)
{
    return getModeFromString(QString::fromStdString(modeStr));
}

QJsonArray Routing::genDomainRules()
{
    QJsonArray domains;

    // 广告域名
    QJsonArray adDomains = getAdDomains();
    for (const auto& domain : adDomains)
    {
        domains.append(domain);
    }

    // 国内域名
    QJsonArray cnDomains = getCnDomains();
    for (const auto& domain : cnDomains)
    {
        domains.append(domain);
    }

    return domains;
}

QJsonArray Routing::genIpRules()
{
    QJsonArray ips;

    // 局域网 IP
    QJsonArray lanIps = getLanIps();
    for (const auto& ip : lanIps)
    {
        ips.append(ip);
    }

    // 国内 IP
    QJsonArray cnIps = getCnIps();
    for (const auto& ip : cnIps)
    {
        ips.append(ip);
    }

    return ips;
}

QJsonArray Routing::genPortRules()
{
    QJsonArray ports;

    // 常用端口
    ports.append("80");
    ports.append("443");

    return ports;
}

QJsonArray Routing::getAdDomains()
{
    QJsonArray domains;
    // 使用 V2Ray 内置的广告域名列表
    domains.append("geosite:category-ads-all");
    return domains;
}

QJsonArray Routing::getCnDomains()
{
    QJsonArray domains;
    // 使用 V2Ray 内置的国内域名列表
    domains.append("geosite:cn");
    domains.append("geosite:geolocation-cn");
    return domains;
}

QJsonArray Routing::getLanIps()
{
    QJsonArray ips;
    // 局域网 IP 段
    ips.append("geoip:private");
    
    // 手动添加的局域网 IP 段（备用）
    ips.append("192.168.0.0/16");
    ips.append("10.0.0.0/8");
    ips.append("172.16.0.0/12");
    ips.append("127.0.0.0/8");
    ips.append("169.254.0.0/16");
    
    return ips;
}

QJsonArray Routing::getCnIps()
{
    QJsonArray ips;
    // 使用 V2Ray 内置的国内 IP 列表
    ips.append("geoip:cn");
    
    // 手动添加的简化版国内 IP 段（备用）
    // 这些是中国主要的 IP 段简写
    ips.append("1.0.1.0/24");
    ips.append("36.152.0.0/24");
    ips.append("42.176.0.0/24");
    ips.append("58.14.0.0/24");
    ips.append("58.16.0.0/24");
    ips.append("60.0.0.0/24");
    ips.append("101.0.0.0/24");
    ips.append("103.0.0.0/24");
    ips.append("106.0.0.0/24");
    ips.append("110.0.0.0/24");
    ips.append("111.0.0.0/24");
    ips.append("112.0.0.0/24");
    ips.append("114.0.0.0/24");
    ips.append("115.0.0.0/24");
    ips.append("116.0.0.0/24");
    ips.append("117.0.0.0/24");
    ips.append("118.0.0.0/24");
    ips.append("119.0.0.0/24");
    ips.append("120.0.0.0/24");
    ips.append("121.0.0.0/24");
    ips.append("122.0.0.0/24");
    ips.append("123.0.0.0/24");
    ips.append("124.0.0.0/24");
    ips.append("125.0.0.0/24");
    ips.append("175.0.0.0/24");
    ips.append("180.0.0.0/24");
    ips.append("182.0.0.0/24");
    ips.append("202.0.0.0/24");
    ips.append("203.0.0.0/24");
    ips.append("210.0.0.0/24");
    ips.append("218.0.0.0/24");
    ips.append("221.0.0.0/24");
    ips.append("222.0.0.0/24");
    
    return ips;
}