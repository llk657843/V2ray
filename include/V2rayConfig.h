#pragma once

#include <string>
#include <vector>
#include "Inbounds.h"
#include "Outbounds.h"

/// <summary>
/// V2Ray 涓婚厤缃枃浠剁粨鏋?/// 瀵瑰簲 V2RayN 鐨?V2rayConfig.cs
/// </summary>
class V2rayConfig
{
public:
    V2rayConfig() = default;
    ~V2rayConfig() = default;

    // Getters
    const std::string& getLogLevel() const { return logLevel; }
    const std::string& getLogAccess() const { return logAccess; }
    const std::string& getLogError() const { return logError; }
    const std::vector<Inbound>& getInbounds() const { return inbounds; }
    const std::vector<Outbound>& getOutbounds() const { return outbounds; }

    // Setters
    void setLogLevel(const std::string& value) { logLevel = value; }
    void setLogAccess(const std::string& value) { logAccess = value; }
    void setLogError(const std::string& value) { logError = value; }
    void addInbound(const Inbound& inbound) { inbounds.push_back(inbound); }
    void addOutbound(const Outbound& outbound) { outbounds.push_back(outbound); }

    /// <summary>
    /// 鍒涘缓榛樿鐨?V2Ray 閰嶇疆锛圫OCKS + HTTP 鍏ョ珯 + Trojan 鍑虹珯锛?    /// </summary>
    /// <param name="profile">ProfileItem 鍖呭惈鍑虹珯閰嶇疆淇℃伅</param>
    /// <returns>V2rayConfig 瀵硅薄</returns>
    static V2rayConfig createDefault(const class ProfileItem& profile);

    /// <summary>
    /// 杞崲涓?JSON 瀛楃涓?    /// </summary>
    std::string toJson() const;

private:
    std::string logLevel = "warning";     // 鏃ュ織绾у埆
    std::string logAccess = "";           // 璁块棶鏃ュ織璺緞
    std::string logError = "";            // 閿欒鏃ュ織璺緞
    std::vector<Inbound> inbounds;        // 鍏ョ珯閰嶇疆鍒楄〃
    std::vector<Outbound> outbounds;      // 鍑虹珯閰嶇疆鍒楄〃
};