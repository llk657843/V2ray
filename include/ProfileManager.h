#pragma once

#include <QString>
#include "ProfileItem.h"

/// 仅提供与节点相关的无状态工具（服务器列表已统一由 Xray config.json 管理，不再使用 servers.json）
class ProfileManager
{
public:
    enum class UrlParseResult { Ok, UnsupportedProtocol, Invalid };

    static UrlParseResult tryParseProfileFromUrl(const QString& url, ProfileItem& profile);
    static bool parseProfileFromUrl(const QString& url, ProfileItem& profile);

private:
    ProfileManager() = delete;
};
