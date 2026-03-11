#pragma once

#include <string>
#include <memory>
#include "ProfileItem.h"

/// <summary>
/// Trojan 协议 URL 解析器
/// 格式: trojan://password@address:port?flow=xtls-rprx-vision&sni=example.com#remark
/// </summary>
class TrojanFmt
{
public:
    TrojanFmt() = default;
    ~TrojanFmt() = default;

    /// <summary>
    /// 解析 Trojan URL 并返回 ProfileItem 对象
    /// </summary>
    /// <param name="url">Trojan URL 字符串</param>
    /// <returns>解析后的 ProfileItem 对象，解析失败返回 nullptr</returns>
    static std::unique_ptr<ProfileItem> parse(const std::string& url);

    /// <summary>
    /// 从 ProfileItem 生成 Trojan URL
    /// </summary>
    /// <param name="item">ProfileItem 对象</param>
    /// <returns>Trojan URL 字符串</returns>
    static std::string generate(const ProfileItem& item);

private:
    /// <summary>
    /// 解析 URL 中的查询参数
    /// </summary>
    static void parseQueryParams(const std::string& query, 
                                   std::string& flow,
                                   std::string& sni,
                                   std::string& security,
                                   std::string& fingerprint,
                                   std::string& alpn,
                                   bool& allowInsecure,
                                   std::string& network);

    /// <summary>
    /// URL 解码
    /// </summary>
    static std::string urlDecode(const std::string& value);
};
