#include "SysProxyHandler.h"
#include <windows.h>
#include <wininet.h>
#include <algorithm>
#include <sstream>

// 注册表路径常量
const wchar_t* SysProxyHandler::REGISTRY_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";

/// <summary>
/// 设置 HTTP 代理
/// </summary>
bool SysProxyHandler::setHttpProxy(const std::string& address, int port, const std::string& bypass)
{
    std::ostringstream oss;
    oss << address << ":" << port;
    std::string proxyServer = oss.str();
    
    return setProxy(proxyServer, bypass, true);
}

/// <summary>
/// 设置 SOCKS 代理
/// </summary>
bool SysProxyHandler::setSocksProxy(const std::string& address, int port, const std::string& bypass)
{
    std::ostringstream oss;
    oss << "socks=" << address << ":" << port;
    std::string proxyServer = oss.str();
    
    return setProxy(proxyServer, bypass, true);
}

/// <summary>
/// 设置代理（通用方法）
/// </summary>
bool SysProxyHandler::setProxy(const std::string& proxyServer, const std::string& bypass, bool enable)
{
    HKEY hKey;
    LONG result;
    
    // 打开注册表项
    result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return false;
    }
    
    // 设置 ProxyEnable
    DWORD proxyEnable = enable ? 1 : 0;
    result = RegSetValueExW(hKey, L"ProxyEnable", 0, REG_DWORD, 
                           reinterpret_cast<const BYTE*>(&proxyEnable), sizeof(proxyEnable));
    
    if (result != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }
    
    // 设置 ProxyServer
    if (enable && !proxyServer.empty())
    {
        std::wstring wProxyServer(proxyServer.begin(), proxyServer.end());
        result = RegSetValueExW(hKey, L"ProxyServer", 0, REG_SZ, 
                               reinterpret_cast<const BYTE*>(wProxyServer.c_str()), 
                               (wProxyServer.length() + 1) * sizeof(wchar_t));
        
        if (result != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return false;
        }
    }
    else
    {
        // 清空 ProxyServer
        result = RegSetValueExW(hKey, L"ProxyServer", 0, REG_SZ, 
                               reinterpret_cast<const BYTE*>(L"\0"), sizeof(wchar_t));
    }
    
    // 设置 ProxyOverride
    std::wstring wBypass(bypass.begin(), bypass.end());
    result = RegSetValueExW(hKey, L"ProxyOverride", 0, REG_SZ, 
                           reinterpret_cast<const BYTE*>(wBypass.c_str()), 
                           (wBypass.length() + 1) * sizeof(wchar_t));
    
    // 清空 AutoConfigURL（如果之前使用 PAC）
    result = RegSetValueExW(hKey, L"AutoConfigURL", 0, REG_SZ, 
                           reinterpret_cast<const BYTE*>(L"\0"), sizeof(wchar_t));
    
    RegCloseKey(hKey);
    
    // 刷新 Internet 设置
    refreshInternetSettings();
    
    return true;
}

/// <summary>
/// 清除代理设置
/// </summary>
bool SysProxyHandler::clearProxy()
{
    return setProxy("", "", false);
}

/// <summary>
/// 设置代理模式
/// </summary>
bool SysProxyHandler::setProxyMode(ESysProxyType mode, const std::string& proxyServer, const std::string& bypass)
{
    switch (mode)
    {
        case ESysProxyType::ForcedClear:
            return clearProxy();
            
        case ESysProxyType::ForcedChange:
            if (proxyServer.empty())
            {
                return false;
            }
            return setProxy(proxyServer, bypass, true);
            
        case ESysProxyType::Pac:
            // PAC 模式需要设置 AutoConfigURL
            {
                HKEY hKey;
                LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_WRITE, &hKey);
                if (result != ERROR_SUCCESS)
                {
                    return false;
                }
                
                // 禁用直接代理
                DWORD proxyEnable = 0;
                result = RegSetValueExW(hKey, L"ProxyEnable", 0, REG_DWORD, 
                                       reinterpret_cast<const BYTE*>(&proxyEnable), sizeof(proxyEnable));
                
                // 清空 ProxyServer
                RegSetValueExW(hKey, L"ProxyServer", 0, REG_SZ, 
                             reinterpret_cast<const BYTE*>(L"\0"), sizeof(wchar_t));
                
                // 设置 PAC URL（这里简化处理，实际需要传入 PAC URL）
                if (!proxyServer.empty())
                {
                    std::wstring wPacUrl(proxyServer.begin(), proxyServer.end());
                    RegSetValueExW(hKey, L"AutoConfigURL", 0, REG_SZ, 
                                 reinterpret_cast<const BYTE*>(wPacUrl.c_str()), 
                                 (wPacUrl.length() + 1) * sizeof(wchar_t));
                }
                
                RegCloseKey(hKey);
                refreshInternetSettings();
                return true;
            }
            
        default:
            return false;
    }
}

/// <summary>
/// 获取当前代理模式
/// </summary>
ESysProxyType SysProxyHandler::getProxyMode() const
{
    DWORD proxyEnable = readRegistryDword(L"ProxyEnable");
    
    if (proxyEnable == 0)
    {
        // 检查是否是 PAC 模式
        std::wstring pacUrl = readRegistryString(L"AutoConfigURL");
        if (!pacUrl.empty())
        {
            return ESysProxyType::Pac;
        }
        return ESysProxyType::ForcedClear;
    }
    
    // 检查是否是全局代理（简单判断：如果包含 socks= 则为手动模式）
    std::wstring proxyServer = readRegistryString(L"ProxyServer");
    if (!proxyServer.empty() && proxyServer.find(L"socks=") != std::wstring::npos)
    {
        // 包含 SOCKS 代理，可能需要更复杂的判断
        // 这里简化 forcedChange 模式
    }
    
    return ESysProxyType::ForcedChange;
}

/// <summary>
/// 获取当前代理服务器地址
/// </summary>
std::string SysProxyHandler::getProxyServer() const
{
    std::wstring wProxyServer = readRegistryString(L"ProxyServer");
    return std::string(wProxyServer.begin(), wProxyServer.end());
}

/// <summary>
/// 检查是否启用了代理
/// </summary>
bool SysProxyHandler::isProxyEnabled() const
{
    DWORD proxyEnable = readRegistryDword(L"ProxyEnable");
    return proxyEnable != 0;
}

/// <summary>
/// 刷新 Internet Settings 使更改生效
/// </summary>
void SysProxyHandler::refreshInternetSettings()
{
    // 通知系统代理设置已更改
    INTERNET_PROXY_INFO* pProxyInfo = nullptr;
    InternetSetOption(nullptr, INTERNET_OPTION_SETTINGS_CHANGED, nullptr, 0);
    InternetSetOption(nullptr, INTERNET_OPTION_REFRESH, nullptr, 0);
}

/// <summary>
/// 写入注册表 DWORD 值
/// </summary>
bool SysProxyHandler::writeRegistryValue(const wchar_t* valueName, DWORD data)
{
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return false;
    }
    
    result = RegSetValueExW(hKey, valueName, 0, REG_DWORD, 
                           reinterpret_cast<const BYTE*>(&data), sizeof(data));
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

/// <summary>
/// 写入注册表字符串值
/// </summary>
bool SysProxyHandler::writeRegistryValue(const wchar_t* valueName, const std::wstring& data)
{
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return false;
    }
    
    result = RegSetValueExW(hKey, valueName, 0, REG_SZ, 
                           reinterpret_cast<const BYTE*>(data.c_str()), 
                           (data.length() + 1) * sizeof(wchar_t));
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

/// <summary>
/// 读取注册表字符串值
/// </summary>
std::wstring SysProxyHandler::readRegistryString(const wchar_t* valueName) const
{
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return L"";
    }
    
    wchar_t buffer[4096];
    DWORD bufferSize = sizeof(buffer);
    DWORD dataType;
    
    result = RegQueryValueExW(hKey, valueName, nullptr, &dataType, 
                             reinterpret_cast<LPBYTE>(buffer), &bufferSize);
    
    RegCloseKey(hKey);
    
    if (result != ERROR_SUCCESS || dataType != REG_SZ)
    {
        return L"";
    }
    
    return std::wstring(buffer);
}

/// <summary>
/// 读取注册表 DWORD 值
/// </summary>
DWORD SysProxyHandler::readRegistryDword(const wchar_t* valueName) const
{
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return 0;
    }
    
    DWORD bufferSize = sizeof(DWORD);
    DWORD dataType;
    DWORD data = 0;
    
    result = RegQueryValueExW(hKey, valueName, nullptr, &dataType, 
                             reinterpret_cast<LPBYTE>(&data), &bufferSize);
    
    RegCloseKey(hKey);
    
    if (result != ERROR_SUCCESS || dataType != REG_DWORD)
    {
        return 0;
    }
    
    return data;
}
