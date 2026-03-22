#include "ProfileManager.h"
#include "TrojanFmt.h"
#include <memory>

ProfileManager::UrlParseResult ProfileManager::tryParseProfileFromUrl(const QString& url, ProfileItem& profile)
{
    const QString trimmed = url.trimmed();
    if (trimmed.startsWith(QStringLiteral("vmess://"), Qt::CaseInsensitive)
        || trimmed.startsWith(QStringLiteral("vless://"), Qt::CaseInsensitive)) {
        return UrlParseResult::UnsupportedProtocol;
    }
    if (trimmed.startsWith(QStringLiteral("trojan://"), Qt::CaseInsensitive)) {
        std::unique_ptr<ProfileItem> parsed = TrojanFmt::parse(trimmed.toStdString());
        if (parsed) {
            profile = *parsed;
            return UrlParseResult::Ok;
        }
        return UrlParseResult::Invalid;
    }
    return UrlParseResult::Invalid;
}

bool ProfileManager::parseProfileFromUrl(const QString& url, ProfileItem& profile)
{
    return tryParseProfileFromUrl(url, profile) == UrlParseResult::Ok;
}
