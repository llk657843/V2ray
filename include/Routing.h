#pragma once

#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include "ProfileItem.h"

/// <summary>
/// 路由模式类型
/// </summary>
enum class ERoutingMode
{
    Proxy = 0,   // 代理全局 - 所有流量走代理
    Direct = 1,  // 直连全局 - 所有流量直连
    Global = 2,  // 全局代理 - 同 proxy
    Rule = 3     // 规则分流（默认）
};

/// <summary>
/// 路由规则类
/// 提供静态方法生成各种路由规则
/// </summary>
class Routing
{
public:
    Routing() = default;
    ~Routing() = default;

    /// <summary>
    /// 生成完整的路由配置
    /// </summary>
    /// <param name="mode">路由模式</param>
    /// <returns>QJsonObject 路由配置</returns>
    static QJsonObject genRouting(ERoutingMode mode = ERoutingMode::Rule);

    /// <summary>
    /// 生成代理全局模式路由（所有流量走代理）
    /// </summary>
    static QJsonObject genRoutingProxy();

    /// <summary>
    /// 生成直连全局模式路由（所有流量直连）
    /// </summary>
    static QJsonObject genRoutingDirect();

    /// <summary>
    /// 生成规则分流模式路由（默认）
    /// </summary>
    static QJsonObject genRoutingRule();

    /// <summary>
    /// 从字符串获取路由模式
    /// </summary>
    /// <param name="modeStr">模式字符串 "proxy"/"direct"/"global"/"rule"</param>
    /// <returns>ERoutingMode</returns>
    static ERoutingMode getModeFromString(const QString& modeStr);

    /// <summary>
    /// 从字符串获取路由模式（重载）
    /// </summary>
    static ERoutingMode getModeFromString(const std::string& modeStr);

private:
    /// <summary>
    /// 生成域名路由规则
    /// </summary>
    static QJsonArray genDomainRules();

    /// <summary>
    /// 生成 IP 路由规则
    /// </summary>
    static QJsonArray genIpRules();

    /// <summary>
    /// 生成端口路由规则
    /// </summary>
    static QJsonArray genPortRules();

    /// <summary>
    /// 生成广告域名列表
    /// </summary>
    static QJsonArray getAdDomains();

    /// <summary>
    /// 生成国内域名列表
    /// </summary>
    static QJsonArray getCnDomains();

    /// <summary>
    /// 生成局域网 IP 列表
    /// </summary>
    static QJsonArray getLanIps();

    /// <summary>
    /// 生成国内 IP 列表（简化版）
    /// </summary>
    static QJsonArray getCnIps();
};
