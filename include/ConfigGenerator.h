#pragma once

#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include "ProfileItem.h"
#include "Routing.h"

/**
 * @brief 配置生成器
/// 将 V2Ray 代理核心配置文件生成 为 JSON 文件
 */
class ConfigGenerator
{
public:
    ConfigGenerator() = default;
    ~ConfigGenerator() = default;

    /**
     * @brief 从 ProfileItem 对象生成 V2Ray 客户端配置 JSON
     * @param node 配置文件节点（包含服务器连接信息）
     * @param routingMode 路由模式字符串 "proxy"/"direct"/"global"/"rule"
     * @return QString JSON 形式的配置文件字符串
     */
    static QString generateClientConfig(const ProfileItem& node, const QString& routingMode = "rule");

    /**
     * @brief 从 ProfileItem 对象生成 V2Ray 客户端配置 JSON（重载）
     * @param node 配置文件节点
     * @param mode 路由模式枚举
     * @return QString JSON 形式的配置文件字符串
     */
    static QString generateClientConfig(const ProfileItem& node, ERoutingMode mode);

    /**
     * @brief 生成配置并写入文件
     * @param node 配置文件节点
     * @param filePath 写入的文件路径
     * @return bool 是否成功
     */
    static bool generateAndWriteConfig(const ProfileItem& node, const QString& filePath);

private:
    /// <summary>
    /// 生成入站配置（SOCKS 10808 + HTTP 10809）
    /// </summary>
    static QJsonArray genInbounds(const ProfileItem& node);

    /// <summary>
    /// 生成出站配置（Trojan + Direct + Block + DNS）
    /// </summary>
    static QJsonArray genOutbounds(const ProfileItem& node);

    /// <summary>
    /// 生成路由规则
    /// </summary>
    static QJsonObject genRouting(ERoutingMode mode);

    /// <summary>
    /// 生成 DNS 配置
    /// </summary>
    static QJsonObject genDns();
};
