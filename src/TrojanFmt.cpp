#include "TrojanFmt.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace
{
    std::string toLower(const std::string& s)
    {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), 
            [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    std::string trim(const std::string& s)
    {
        auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
        auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); }).base();
        return (start < end) ? std::string(start, end) : std::string();
    }
}

std::unique_ptr<ProfileItem> TrojanFmt::parse(const std::string& url)
{
    if (url.empty() || toLower(url).find("trojan://") != 0)
    {
        return nullptr;
    }

    std::string remaining = url.substr(9);

    std::string mainPart = remaining;
    std::string remark;
    size_t hashPos = remaining.find('#');
    if (hashPos != std::string::npos)
    {
        mainPart = remaining.substr(0, hashPos);
        remark = remaining.substr(hashPos + 1);
        remark = urlDecode(remark);
    }

    size_t queryPos = mainPart.find('?');
    std::string userInfoPart;
    std::string queryPart;

    if (queryPos != std::string::npos)
    {
        userInfoPart = mainPart.substr(0, queryPos);
        queryPart = mainPart.substr(queryPos + 1);
    }
    else
    {
        userInfoPart = mainPart;
    }

    size_t atPos = userInfoPart.find('@');
    if (atPos == std::string::npos)
    {
        return nullptr;
    }

    std::string password = userInfoPart.substr(0, atPos);
    password = urlDecode(password);
    std::string hostPort = userInfoPart.substr(atPos + 1);

    size_t colonPos = hostPort.find(':');
    if (colonPos == std::string::npos)
    {
        return nullptr;
    }

    std::string address = hostPort.substr(0, colonPos);
    std::string portStr = hostPort.substr(colonPos + 1);

    int port = 0;
    try
    {
        port = std::stoi(portStr);
    }
    catch (...)
    {
        return nullptr;
    }

    std::string flow, sni, security, fingerprint, alpn, network;
    bool allowInsecure = false;
    parseQueryParams(queryPart, flow, sni, security, fingerprint, alpn, allowInsecure, network);

    auto item = std::make_unique<ProfileItem>();
    item->setConfigType(EConfigType::Trojan);
    item->setAddress(address);
    item->setPort(port);
    item->setPassword(password);
    item->setRemark(remark);
    item->setFlow(flow);
    item->setSni(sni);
    
    if (!security.empty())
    {
        item->setSecurity(security);
    }
    else
    {
        item->setSecurity("tls");
    }
    
    item->setFingerprint(fingerprint);
    item->setAlpn(alpn);
    item->setAllowInsecure(allowInsecure);
    
    if (!network.empty())
    {
        item->setNetwork(network);
    }

    return item;
}

void TrojanFmt::parseQueryParams(const std::string& query, 
                                   std::string& flow,
                                   std::string& sni,
                                   std::string& security,
                                   std::string& fingerprint,
                                   std::string& alpn,
                                   bool& allowInsecure,
                                   std::string& network)
{
    if (query.empty())
    {
        return;
    }

    std::stringstream ss(query);
    std::string param;

    while (std::getline(ss, param, '&'))
    {
        size_t eqPos = param.find('=');
        if (eqPos == std::string::npos)
        {
            continue;
        }

        std::string key = toLower(trim(param.substr(0, eqPos)));
        std::string value = param.substr(eqPos + 1);
        value = urlDecode(value);

        if (key == "flow")
        {
            flow = value;
        }
        else if (key == "sni")
        {
            sni = value;
        }
        else if (key == "security")
        {
            security = toLower(value);
        }
        else if (key == "fp" || key == "fingerprint")
        {
            fingerprint = value;
        }
        else if (key == "alpn")
        {
            alpn = value;
        }
        else if (key == "allowinsecure" || key == "allow_insecure")
        {
            allowInsecure = (toLower(value) == "1" || toLower(value) == "true" || toLower(value) == "yes");
        }
        else if (key == "network" || key == "type")
        {
            network = toLower(value);
        }
    }
}

std::string TrojanFmt::generate(const ProfileItem& item)
{
    std::ostringstream url;
    url << "trojan://";
    url << item.getPassword();
    url << "@" << item.getAddress() << ":" << item.getPort();

    bool hasQuery = false;
    
    if (!item.getFlow().empty())
    {
        url << (hasQuery ? "&" : "?");
        url << "flow=" << item.getFlow();
        hasQuery = true;
    }

    if (!item.getSni().empty())
    {
        url << (hasQuery ? "&" : "?");
        url << "sni=" << item.getSni();
        hasQuery = true;
    }

    if (!item.getSecurity().empty() && item.getSecurity() != "tls")
    {
        url << (hasQuery ? "&" : "?");
        url << "security=" << item.getSecurity();
        hasQuery = true;
    }

    if (!item.getFingerprint().empty())
    {
        url << (hasQuery ? "&" : "?");
        url << "fp=" << item.getFingerprint();
        hasQuery = true;
    }

    if (!item.getAlpn().empty())
    {
        url << (hasQuery ? "&" : "?");
        url << "alpn=" << item.getAlpn();
        hasQuery = true;
    }

    if (item.getAllowInsecure())
    {
        url << (hasQuery ? "&" : "?");
        url << "allowInsecure=1";
        hasQuery = true;
    }

    if (!item.getNetwork().empty())
    {
        url << (hasQuery ? "&" : "?");
        url << "network=" << item.getNetwork();
    }

    if (!item.getRemark().empty())
    {
        url << "#" << item.getRemark();
    }

    return url.str();
}

std::string TrojanFmt::urlDecode(const std::string& value)
{
    if (value.empty())
    {
        return value;
    }

    std::string result;
    result.reserve(value.size());

    for (size_t i = 0; i < value.size(); ++i)
    {
        if (value[i] == '%' && i + 2 < value.size())
        {
            std::string hex = value.substr(i + 1, 2);
            try
            {
                char ch = static_cast<char>(std::stoi(hex, nullptr, 16));
                result += ch;
                i += 2;
            }
            catch (...)
            {
                result += value[i];
            }
        }
        else if (value[i] == '+')
        {
            result += ' ';
        }
        else
        {
            result += value[i];
        }
    }

    return result;
}
