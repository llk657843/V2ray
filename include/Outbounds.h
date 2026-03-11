#pragma once

#include <string>
#include <vector>

/// <summary>
/// TLS 璁剧疆
/// 瀵瑰簲 C# 鐨?TlsSettings4Ray
/// </summary>
class TlsSettings
{
public:
    TlsSettings() = default;
    ~TlsSettings() = default;

    bool isAllowInsecure() const { return allowInsecure; }
    void setAllowInsecure(bool value) { allowInsecure = value; }
    
    const std::string& getServerName() const { return serverName; }
    void setServerName(const std::string& value) { serverName = value; }
    
    const std::vector<std::string>& getAlpn() const { return alpn; }
    void addAlpn(const std::string& value) { alpn.push_back(value); }
    
    const std::string& getFingerprint() const { return fingerprint; }
    void setFingerprint(const std::string& value) { fingerprint = value; }

    std::string toJson() const;

private:
    bool allowInsecure = false;
    std::string serverName;
    std::vector<std::string> alpn;
    std::string fingerprint;
};

/// <summary>
/// TCP 璁剧疆
/// 瀵瑰簲 C# 鐨?TcpSettings4Ray
/// </summary>
class TcpSettings
{
public:
    TcpSettings() = default;
    ~TcpSettings() = default;

    std::string toJson() const;
};

/// <summary>
/// 娴佽缃?/// 瀵瑰簲 C# 鐨?StreamSettings4Ray
/// </summary>
class StreamSettings
{
public:
    StreamSettings() = default;
    ~StreamSettings() = default;

    const std::string& getNetwork() const { return network; }
    void setNetwork(const std::string& value) { network = value; }
    
    const std::string& getSecurity() const { return security; }
    void setSecurity(const std::string& value) { security = value; }
    
    const TlsSettings& getTlsSettings() const { return tlsSettings; }
    void setTlsSettings(const TlsSettings& value) { tlsSettings = value; }
    
    const TcpSettings& getTcpSettings() const { return tcpSettings; }
    void setTcpSettings(const TcpSettings& value) { tcpSettings = value; }

    static StreamSettings createTrojan(const std::string& sni, const std::string& fingerprint = "");

    std::string toJson() const;

private:
    std::string network = "tcp";
    std::string security = "tls";
    TlsSettings tlsSettings;
    TcpSettings tcpSettings;
};

/// <summary>
/// Mux 璁剧疆
/// 瀵瑰簲 C# 鐨?Mux4Ray
/// </summary>
class MuxSettings
{
public:
    MuxSettings() = default;
    ~MuxSettings() = default;

    bool isEnabled() const { return enabled; }
    void setEnabled(bool value) { enabled = value; }
    
    int getConcurrency() const { return concurrency; }
    void setConcurrency(int value) { concurrency = value; }

    std::string toJson() const;

private:
    bool enabled = false;
    int concurrency = 8;
};

/// <summary>
/// Trojan 鏈嶅姟鍣ㄩ厤缃?/// 瀵瑰簲 C# 鐨?ServersItem4Ray
/// </summary>
class TrojanServer
{
public:
    TrojanServer() = default;
    ~TrojanServer() = default;

    const std::string& getAddress() const { return address; }
    void setAddress(const std::string& value) { address = value; }
    
    int getPort() const { return port; }
    void setPort(int value) { port = value; }
    
    const std::string& getPassword() const { return password; }
    void setPassword(const std::string& value) { password = value; }
    
    const std::string& getFlow() const { return flow; }
    void setFlow(const std::string& value) { flow = value; }
    
    int getLevel() const { return level; }
    void setLevel(int value) { level = value; }

    std::string toJson() const;

private:
    std::string address;
    int port = 0;
    std::string password;
    std::string flow;
    int level = 0;
};

/// <summary>
/// 鍑虹珯璁剧疆
/// 瀵瑰簲 C# 鐨?Outboundsettings4Ray
/// </summary>
class OutboundSettings
{
public:
    OutboundSettings() = default;
    ~OutboundSettings() = default;

    const std::vector<TrojanServer>& getServers() const { return servers; }
    void addServer(const TrojanServer& server) { servers.push_back(server); }

    std::string toJson() const;

private:
    std::vector<TrojanServer> servers;
};

/// <summary>
/// 鍑虹珯閰嶇疆
/// 瀵瑰簲 C# 鐨?Outbounds4Ray
/// </summary>
class Outbound
{
public:
    Outbound() = default;
    ~Outbound() = default;

    const std::string& getTag() const { return tag; }
    const std::string& getProtocol() const { return protocol; }
    const OutboundSettings& getSettings() const { return settings; }
    const StreamSettings& getStreamSettings() const { return streamSettings; }
    const MuxSettings& getMux() const { return mux; }

    void setTag(const std::string& value) { tag = value; }
    void setProtocol(const std::string& value) { protocol = value; }
    void setSettings(const OutboundSettings& value) { settings = value; }
    void setStreamSettings(const StreamSettings& value) { streamSettings = value; }
    void setMux(const MuxSettings& value) { mux = value; }

    static Outbound createTrojan(
        const std::string& address,
        int port,
        const std::string& password,
        const std::string& flow,
        const std::string& sni,
        const std::string& fingerprint = "");

    std::string toJson() const;

private:
    std::string tag;
    std::string protocol = "trojan";
    OutboundSettings settings;
    StreamSettings streamSettings;
    MuxSettings mux;
};