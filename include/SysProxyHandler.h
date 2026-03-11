#pragma once
#include <string>
#include <windows.h>
#include "AppConfig.h"

/// <summary>
/// 系统代理设置处理器
/// 操作 Windows 注册表设置/清除代理
/// </summary>
class SysProxyHandler
{
public:
    /// <summary>
    /// 默认 HTTP 代理端口
    /// </summary>
    static const int DEFAULT_HTTP_PORT = 10809;
    
    /// <summary>
    /// 默认 SOCKS 代理端口
    /// </summary>
    static const int DEFAULT_SOCKS_PORT = 10808;
    
    SysProxyHandler() = default;
    ~SysProxyHandler() = default;
    
    /// <summary>
    /// 设置 HTTP 代理
    /// </summary>
    /// <param name="address">代理服务器地址</param>
    /// <param name="port">代理端口</param>
    /// <param name="bypass">绕过代理的地址列表（可选）</param>
    /// <returns>是否设置成功</returns>
    bool setHttpProxy(const std::string& address, int port, const std::string& bypass = "");
    
    /// <summary>
    /// 设置 SOCKS 代理
    /// </summary>
    /// <param name="address">代理服务器地址</param>
    /// <param name="port">代理端口</param>
    /// <param name="bypass">绕过代理的地址列表（可选）</param>
    /// <returns>是否设置成功</returns>
    bool setSocksProxy(const std::string& address, int port, const std::string& bypass = "");
    
    /// <summary>
    /// 设置代理（通用方法）
    /// </summary>
    /// <param name="proxyServer">代理服务器地址，格式如 "127.0.0.1:10809"</param>
    /// <param name="bypass">绕过代理的地址列表</param>
    /// <param name="enable">是否启用代理</param>
    /// <returns>是否设置成功</returns>
    bool setProxy(const std::string& proxyServer, const std::string& bypass = "", bool enable = true);
    
    /// <summary>
    /// 清除代理设置
    /// </summary>
    /// <returns>是否清除成功</returns>
    bool clearProxy();
    
    /// <summary>
    /// 设置代理模式
    /// </summary>
    /// <param name="mode">代理模式 (ESysProxyType)</param>
    /// <param name="proxyServer">代理服务器地址（ForcedChange 模式需要）</param>
    /// <param name="bypass">绕过代理的地址列表</param>
    /// <returns>是否设置成功</returns>
    bool setProxyMode(ESysProxyType mode, const std::string& proxyServer = "", const std::string& bypass = "");
    
    /// <summary>
    /// 获取当前代理模式
    /// </summary>
    /// <returns>当前代理模式</returns>
    ESysProxyType getProxyMode() const;
    
    /// <summary>
    /// 获取当前代理服务器地址
    /// </summary>
    /// <returns>代理服务器地址</returns>
    std::string getProxyServer() const;
    
    /// <summary>
    /// 检查是否启用了代理
    /// </summary>
    /// <returns>是否启用代理</returns>
    bool isProxyEnabled() const;
    
    /// <summary>
    /// 刷新 Internet Settings 使更改生效
    /// </summary>
    void refreshInternetSettings();
    
private:
    /// <summary>
    /// 注册表路径
    /// </summary>
    static const wchar_t* REGISTRY_PATH;
    
    /// <summary>
    /// 写入注册表值
    /// </summary>
    bool writeRegistryValue(const wchar_t* valueName, DWORD data);
    bool writeRegistryValue(const wchar_t* valueName, const std::wstring& data);
    
    /// <summary>
    /// 读取注册表值
    /// </summary>
    std::wstring readRegistryString(const wchar_t* valueName) const;
    DWORD readRegistryDword(const wchar_t* valueName) const;
};

