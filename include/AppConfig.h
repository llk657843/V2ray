#pragma once

#include <string>
#include <vector>
#include <QString>
#include <QJsonObject>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

/// <summary>
/// 系统代理类型枚举（用于系统代理设置）
/// 参考 V2RayN ESysProxyType
/// </summary>
enum class ESysProxyType
{
    ForcedClear = 0,    // 清除代理
    ForcedChange = 1,   // 强制修改代理
    Unchanged = 2,      // 不改变
    Pac = 3             // PAC模式
};

/// <summary>
/// 代理模式枚举（用于路由规则）
/// </summary>
enum class ProxyMode
{
    None = 0,      // 不使用代理
    Proxy = 1,     // 代理模式
    Direct = 2,    // 直连模式
    Global = 3     // 全局模式
};

/// <summary>
/// 核心配置项 (CoreBasicItem)
/// </summary>
struct CoreBasicItem
{
    bool logEnabled = true;                    // 是否启用日志
    std::string logLevel = "warning";          // 日志级别: debug, info, warning, error
    bool muxEnabled = false;                   // 是否启用 Mux
    bool defAllowInsecure = false;             // 默认允许不安全证书
    std::string defFingerprint;                // 默认指纹
    std::string defUserAgent;                  // 默认 User-Agent

    // JSON 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

/// <summary>
/// 入站配置项 (InboundItem)
/// </summary>
struct InboundItem
{
    int localPort = 10808;                     // 本地监听端口
    std::string protocol = "socks";            // 协议: socks, http
    bool udpEnabled = true;                    // 是否启用 UDP
    bool sniffingEnabled = true;               // 是否启用流量探测
    std::vector<std::string> destOverride = {"http", "tls"};  // 目标覆盖协议
    bool routeOnly = false;                    // 仅路由
    bool allowLANConn = false;                 // 允许局域网连接
    std::string user;                          // 用户名
    std::string pass;                          // 密码

    // JSON 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

/// <summary>
/// 路由配置项 (RoutingItem)
/// </summary>
struct RoutingItem
{
    std::string mode = "proxy";                // 路由模式: proxy, direct, global
    std::string domainStrategy = "IPIfNonMatch"; // 域名策略: AsIs, IPIfNonMatch, IPAlways

    // JSON 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

/// <summary>
/// UI配置项 (GUIItem)
/// </summary>
struct GUIItem
{
    bool autoRun = false;                      // 开机自启
    bool enableStatistics = false;             // 启用流量统计
    bool displayRealTimeSpeed = false;         // 显示实时速度

    // JSON 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

/// <summary>
/// 系统代理配置项
/// </summary>
struct SystemProxyItem
{
    bool systemProxyEnabled = false;           // 启用系统代理
    ProxyMode proxyMode = ProxyMode::Proxy;    // 代理模式
    std::string systemProxyExceptions;          // 代理例外地址
    bool notProxyLocalAddress = true;          // 不代理本地地址

    // JSON 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

/// <summary>
/// 应用配置管理类
/// 负责加载、保存和管理应用程序配置
/// </summary>
class AppConfig
{
public:
    /// <summary>
    /// 获取单例实例
    /// </summary>
    static AppConfig& instance()
    {
        static AppConfig config;
        return config;
    }

    // ============ CoreBasicItem 配置 ============
    
    // 日志相关
    void setLogEnabled(bool enabled) { m_coreBasic.logEnabled = enabled; }
    bool isLogEnabled() const { return m_coreBasic.logEnabled; }
    
    void setLogLevel(const std::string& level) { m_coreBasic.logLevel = level; }
    std::string getLogLevel() const { return m_coreBasic.logLevel; }
    
    // Mux 设置
    void setMuxEnabled(bool enabled) { m_coreBasic.muxEnabled = enabled; }
    bool isMuxEnabled() const { return m_coreBasic.muxEnabled; }
    
    // 安全设置
    void setDefAllowInsecure(bool allow) { m_coreBasic.defAllowInsecure = allow; }
    bool isDefAllowInsecure() const { return m_coreBasic.defAllowInsecure; }
    
    void setDefFingerprint(const std::string& fp) { m_coreBasic.defFingerprint = fp; }
    std::string getDefFingerprint() const { return m_coreBasic.defFingerprint; }
    
    void setDefUserAgent(const std::string& ua) { m_coreBasic.defUserAgent = ua; }
    std::string getDefUserAgent() const { return m_coreBasic.defUserAgent; }
    
    // ============ InboundItem 配置 ============
    
    void setLocalPort(int port) { m_inbound.localPort = port; }
    int getLocalPort() const { return m_inbound.localPort; }
    
    void setProtocol(const std::string& protocol) { m_inbound.protocol = protocol; }
    std::string getProtocol() const { return m_inbound.protocol; }
    
    void setUdpEnabled(bool enabled) { m_inbound.udpEnabled = enabled; }
    bool isUdpEnabled() const { return m_inbound.udpEnabled; }
    
    void setSniffingEnabled(bool enabled) { m_inbound.sniffingEnabled = enabled; }
    bool isSniffingEnabled() const { return m_inbound.sniffingEnabled; }
    
    // ============ RoutingItem 配置 ============
    
    void setRoutingMode(const std::string& mode) { m_routing.mode = mode; }
    std::string getRoutingMode() const { return m_routing.mode; }
    
    void setDomainStrategy(const std::string& strategy) { m_routing.domainStrategy = strategy; }
    std::string getDomainStrategy() const { return m_routing.domainStrategy; }
    
    // ============ GUIItem 配置 ============
    
    void setAutoRun(bool autoRun) { m_gui.autoRun = autoRun; }
    bool isAutoRun() const { return m_gui.autoRun; }
    
    void setEnableStatistics(bool enabled) { m_gui.enableStatistics = enabled; }
    bool isEnableStatistics() const { return m_gui.enableStatistics; }
    
    void setDisplayRealTimeSpeed(bool display) { m_gui.displayRealTimeSpeed = display; }
    bool isDisplayRealTimeSpeed() const { return m_gui.displayRealTimeSpeed; }
    
    // ============ SystemProxyItem 配置 ============
    
    void setSystemProxyEnabled(bool enabled) { m_systemProxy.systemProxyEnabled = enabled; }
    bool isSystemProxyEnabled() const { return m_systemProxy.systemProxyEnabled; }
    
    void setProxyMode(ProxyMode mode) { m_systemProxy.proxyMode = mode; }
    ProxyMode getProxyMode() const { return m_systemProxy.proxyMode; }
    
    void setSystemProxyExceptions(const std::string& exceptions) { m_systemProxy.systemProxyExceptions = exceptions; }
    std::string getSystemProxyExceptions() const { return m_systemProxy.systemProxyExceptions; }
    
    // ============ Xray-core 路径配置 (兼容旧接口) ============
    
    void setCorePath(const QString& path) { m_corePath = path; }
    QString getCorePath() const { return m_corePath; }
    
    void setCoreConfigPath(const QString& path) { m_coreConfigPath = path; }
    QString getCoreConfigPath() const { return m_coreConfigPath; }
    
    void setLogFilePath(const QString& path) { m_logFilePath = path; }
    QString getLogFilePath() const { return m_logFilePath; }
    
    // 默认路径
    QString getDefaultCorePath() const;
    QString getDefaultConfigPath() const;
    
    // ============ 配置管理方法 ============
    
    /// <summary>
    /// 获取配置目录路径
    /// </summary>
    /// <returns>配置目录路径</returns>
    QString getConfigPath() const;
    
    /// <summary>
    /// 获取配置文件路径
    /// </summary>
    /// <returns>配置文件完整路径</returns>
    QString getConfigFilePath() const;
    
    /// <summary>
    /// 加载配置文件
    /// </summary>
    /// <returns>true if load successful</returns>
    bool load();
    
    /// <summary>
    /// 保存配置文件
    /// </summary>
    /// <returns>true if save successful</returns>
    bool save();
    
    /// <summary>
    /// 重置为默认配置
    /// </summary>
    void resetToDefault();
    
    /// <summary>
    /// 获取完整配置对象引用
    /// </summary>
    CoreBasicItem& getCoreBasic() { return m_coreBasic; }
    const CoreBasicItem& getCoreBasic() const { return m_coreBasic; }
    
    InboundItem& getInbound() { return m_inbound; }
    const InboundItem& getInbound() const { return m_inbound; }
    
    RoutingItem& getRouting() { return m_routing; }
    const RoutingItem& getRouting() const { return m_routing; }
    
    GUIItem& getGUI() { return m_gui; }
    const GUIItem& getGUI() const { return m_gui; }
    
    SystemProxyItem& getSystemProxy() { return m_systemProxy; }
    const SystemProxyItem& getSystemProxy() const { return m_systemProxy; }

private:
    AppConfig() = default;
    ~AppConfig() = default;

    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;

    // 配置项
    CoreBasicItem m_coreBasic;      // 核心设置
    InboundItem m_inbound;         // 入站设置
    RoutingItem m_routing;         // 路由设置
    GUIItem m_gui;                  // UI设置
    SystemProxyItem m_systemProxy;  // 系统代理设置
    
    // Xray-core 路径配置 (兼容旧接口)
    QString m_corePath;
    QString m_coreConfigPath;
    QString m_logFilePath;
};
