#pragma once

#include <string>

/// <summary>
/// 鑺傜偣閰嶇疆绫诲瀷鏋氫妇
/// </summary>
enum class EConfigType
{
    VMess = 1,
    VLESS = 2,
    Trojan = 3,
    Shadowsocks = 4,
    Socks = 5,
    Http = 6,
    Hysteria2 = 7,
    Tuic = 8,
    Wireguard = 9,
    Singbox = 10
};

/// <summary>
/// 鍗忚棰濆鍙傛暟锛堢敤浜庡瓨鍌?flow 鍙婂叾浠栨墿灞曞弬鏁帮級
/// </summary>
struct ProtocolExtra
{
    std::string flow;
    std::string method;
    std::string network;
    std::string security;
    std::string sni;
    std::string fingerprint;
    std::string alpn;
    bool allowInsecure = false;
    int seed = 0;
};

/// <summary>
/// 鑺傜偣閰嶇疆鏂囦欢椤癸紙ProfileItem锛?/// 瀛樺偍鎵€鏈夊崟涓妭鐐硅繛鎺ラ厤缃俊鎭?/// </summary>
class ProfileItem
{
public:
    ProfileItem() = default;
    ~ProfileItem() = default;

    std::string getAddress() const { return address; }
    int getPort() const { return port; }
    std::string getPassword() const { return password; }
    std::string getFlow() const { return flow; }
    std::string getRemark() const { return remark; }
    EConfigType getConfigType() const { return configType; }
    std::string getUserId() const { return userId; }
    std::string getAlterId() const { return alterId; }
    std::string getNetwork() const { return network; }
    std::string getSecurity() const { return security; }
    std::string getSni() const { return sni; }
    std::string getFingerprint() const { return fingerprint; }
    std::string getAlpn() const { return alpn; }
    bool getAllowInsecure() const { return allowInsecure; }
    std::string getPublicKey() const { return publicKey; }
    std::string getShortId() const { return shortId; }
    std::string getSpiderX() const { return spiderX; }
    ProtocolExtra getProtocolExtra() const { return protocolExtra; }

    void setAddress(const std::string& value) { address = value; }
    void setPort(int value) { port = value; }
    void setPassword(const std::string& value) { password = value; }
    void setFlow(const std::string& value) { flow = value; }
    void setRemark(const std::string& value) { remark = value; }
    void setConfigType(EConfigType value) { configType = value; }
    void setUserId(const std::string& value) { userId = value; }
    void setAlterId(const std::string& value) { alterId = value; }
    void setNetwork(const std::string& value) { network = value; }
    void setSecurity(const std::string& value) { security = value; }
    void setSni(const std::string& value) { sni = value; }
    void setFingerprint(const std::string& value) { fingerprint = value; }
    void setAlpn(const std::string& value) { alpn = value; }
    void setAllowInsecure(bool value) { allowInsecure = value; }
    void setPublicKey(const std::string& value) { publicKey = value; }
    void setShortId(const std::string& value) { shortId = value; }
    void setSpiderX(const std::string& value) { spiderX = value; }
    void setProtocolExtra(const ProtocolExtra& value) { protocolExtra = value; }

    bool isValid() const;
    std::string getConfigTypeString() const;

private:
    std::string address;
    int port = 0;
    std::string password;
    std::string flow;
    std::string remark;
    EConfigType configType = EConfigType::Trojan;

    std::string userId;
    std::string alterId;

    std::string network;
    std::string security;
    std::string sni;
    std::string fingerprint;
    std::string alpn;
    bool allowInsecure = false;

    std::string publicKey;
    std::string shortId;
    std::string spiderX;

    ProtocolExtra protocolExtra;
};