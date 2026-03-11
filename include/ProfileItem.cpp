#include "ProfileItem.h"
#include <algorithm>

bool ProfileItem::isValid() const
{
    // 基本验证：地址和端口必填
    if (address.empty()) {
        return false;
    }
    
    if (port <= 0 || port > 65535) {
        return false;
    }
    
    // 不同类型有不同的必填字段
    switch (configType) {
        case EConfigType::Trojan:
        case EConfigType::VLESS:
            return !password.empty();
            
        case EConfigType::VMess:
            return !userId.empty();
            
        case EConfigType::Shadowsocks:
        case EConfigType::Socks:
        case EConfigType::Http:
            return !password.empty();
            
        default:
            return true;
    }
}

std::string ProfileItem::getConfigTypeString() const
{
    switch (configType) {
        case EConfigType::VMess:       return "vmess";
        case EConfigType::VLESS:       return "vless";
        case EConfigType::Trojan:      return "trojan";
        case EConfigType::Shadowsocks: return "shadowsocks";
        case EConfigType::Socks:       return "socks";
        case EConfigType::Http:        return "http";
        case EConfigType::Hysteria2:   return "hysteria2";
        case EConfigType::Tuic:        return "tuic";
        case EConfigType::Wireguard:   return "wireguard";
        case EConfigType::Singbox:     return "singbox";
        default:                        return "unknown";
    }
}
