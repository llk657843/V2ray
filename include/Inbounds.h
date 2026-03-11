#pragma once

#include <string>
#include <vector>

/// <summary>
/// 鍏ョ珯璁剧疆锛圫niffing锛?/// 瀵瑰簲 C# 鐨?Sniffing4Ray
/// </summary>
class Sniffing
{
public:
    Sniffing() = default;
    ~Sniffing() = default;

    bool isEnabled() const { return enabled; }
    void setEnabled(bool value) { enabled = value; }
    
    const std::vector<std::string>& getDestOverride() const { return destOverride; }
    void addDestOverride(const std::string& value) { destOverride.push_back(value); }
    
    bool isRouteOnly() const { return routeOnly; }
    void setRouteOnly(bool value) { routeOnly = value; }

    std::string toJson() const;

private:
    bool enabled = true;
    std::vector<std::string> destOverride;
    bool routeOnly = false;
};

/// <summary>
/// 鍏ョ珯鐢ㄦ埛锛圫OCKS/HTTP 璁よ瘉锛?/// 瀵瑰簲 C# 鐨?UsersItem4Ray
/// </summary>
class InboundUser
{
public:
    InboundUser() = default;
    ~InboundUser() = default;

    const std::string& getUser() const { return user; }
    void setUser(const std::string& value) { user = value; }
    
    const std::string& getPass() const { return pass; }
    void setPass(const std::string& value) { pass = value; }
    
    int getLevel() const { return level; }
    void setLevel(int value) { level = value; }

    std::string toJson() const;

private:
    std::string user;
    std::string pass;
    int level = 0;
};

/// <summary>
/// 鍏ョ珯璁剧疆
/// 瀵瑰簲 C# 鐨?Inboundsettings4Ray
/// </summary>
class InboundSettings
{
public:
    InboundSettings() = default;
    ~InboundSettings() = default;

    const std::string& getAuth() const { return auth; }
    void setAuth(const std::string& value) { auth = value; }
    
    bool isUdp() const { return udp; }
    void setUdp(bool value) { udp = value; }
    
    const std::string& getIp() const { return ip; }
    void setIp(const std::string& value) { ip = value; }
    
    const std::string& getAddress() const { return address; }
    void setAddress(const std::string& value) { address = value; }
    
    const std::vector<InboundUser>& getClients() const { return clients; }
    void addClient(const InboundUser& client) { clients.push_back(client); }
    
    bool isAllowTransparent() const { return allowTransparent; }
    void setAllowTransparent(bool value) { allowTransparent = value; }

    std::string toJson() const;

private:
    std::string auth = "noauth";      // SOCKS: noauth, HTTP: password
    bool udp = true;
    std::string ip;
    std::string address;
    std::vector<InboundUser> clients;
    bool allowTransparent = false;
};

/// <summary>
/// 鍏ョ珯閰嶇疆
/// 瀵瑰簲 C# 鐨?Inbounds4Ray
/// </summary>
class Inbound
{
public:
    Inbound() = default;
    ~Inbound() = default;

    // Getters
    const std::string& getTag() const { return tag; }
    int getPort() const { return port; }
    const std::string& getListen() const { return listen; }
    const std::string& getProtocol() const { return protocol; }
    const Sniffing& getSniffing() const { return sniffing; }
    const InboundSettings& getSettings() const { return settings; }

    // Setters
    void setTag(const std::string& value) { tag = value; }
    void setPort(int value) { port = value; }
    void setListen(const std::string& value) { listen = value; }
    void setProtocol(const std::string& value) { protocol = value; }
    void setSniffing(const Sniffing& value) { sniffing = value; }
    void setSettings(const InboundSettings& value) { settings = value; }

    /// <summary>
    /// 鍒涘缓 SOCKS 鍏ョ珯閰嶇疆
    /// </summary>
    static Inbound createSocks(int port, const std::string& listen = "127.0.0.1");

    /// <summary>
    /// 鍒涘缓 HTTP 鍏ョ珯閰嶇疆
    /// </summary>
    static Inbound createHttp(int port, const std::string& listen = "127.0.0.1");

    std::string toJson() const;

private:
    std::string tag;
    int port = 0;
    std::string listen = "127.0.0.1";
    std::string protocol;
    Sniffing sniffing;
    InboundSettings settings;
};