#include "XrayConfigStore.h"
#include "ProfileItem.h"
#include "AppConfig.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDebug>

bool XrayConfigStore::loadServerProfiles(const QString& configFilePath, std::vector<ProfileItem>& out,
                                         QString* errorMessage)
{
    QFile file(configFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "[DEBUG] No config.json found at path:" << configFilePath;
        return true;
    }

    QByteArray data = file.readAll();
    file.close();

    qDebug() << "[DEBUG] config.json content size:" << data.size();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage) {
            *errorMessage = parseError.errorString();
        }
        qWarning() << "[DEBUG] Failed to parse config.json:" << parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("config.json is not a valid JSON object");
        }
        qWarning() << "[DEBUG] config.json is not a valid JSON object";
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray outbounds = root["outbounds"].toArray();

    std::vector<ProfileItem> tmp;

    for (int i = 0; i < outbounds.size(); ++i) {
        QJsonObject outbound = outbounds[i].toObject();
        QString protocol = outbound["protocol"].toString();
        QString tag = outbound["tag"].toString();

        if (tag == "direct" || tag == "block" || tag == "dns-outbound" || tag.isEmpty()) {
            continue;
        }

        QJsonObject settings = outbound["settings"].toObject();
        QJsonObject streamSettings = outbound["streamSettings"].toObject();

        ProfileItem profile;
        profile.setRemark(tag.toStdString());

        if (protocol == "trojan") {
            QJsonArray servers = settings["servers"].toArray();
            if (servers.size() > 0) {
                QJsonObject server = servers[0].toObject();
                profile.setAddress(server["address"].toString().toStdString());
                profile.setPort(server["port"].toInt());
                profile.setPassword(server["password"].toString().toStdString());

                QString flow = server["flow"].toString();
                if (!flow.isEmpty()) {
                    profile.setFlow(flow.toStdString());
                }

                profile.setConfigType(EConfigType::Trojan);
            }
        } else if (protocol == "vmess") {
            QJsonArray vnext = settings["vnext"].toArray();
            if (vnext.size() > 0) {
                QJsonObject server = vnext[0].toObject();
                profile.setAddress(server["address"].toString().toStdString());
                profile.setPort(server["port"].toInt());

                QJsonArray users = server["users"].toArray();
                if (users.size() > 0) {
                    QJsonObject user = users[0].toObject();
                    profile.setUserId(user["id"].toString().toStdString());
                    profile.setSecurity(user["security"].toString().toStdString());
                }
                profile.setConfigType(EConfigType::VMess);
            }
        } else if (protocol == "vless") {
            QJsonArray vnext = settings["vnext"].toArray();
            if (vnext.size() > 0) {
                QJsonObject server = vnext[0].toObject();
                profile.setAddress(server["address"].toString().toStdString());
                profile.setPort(server["port"].toInt());

                QJsonArray users = server["users"].toArray();
                if (users.size() > 0) {
                    QJsonObject user = users[0].toObject();
                    profile.setUserId(user["id"].toString().toStdString());

                    QString flow = user["flow"].toString();
                    if (!flow.isEmpty()) {
                        profile.setFlow(flow.toStdString());
                    }
                }
                profile.setConfigType(EConfigType::VLESS);
            }
        } else {
            continue;
        }

        QString network = streamSettings["network"].toString("tcp");
        QString security = streamSettings["security"].toString("");
        profile.setNetwork(network.toStdString());
        profile.setSecurity(security.toStdString());

        if (security == "tls") {
            QJsonObject tlsSettings = streamSettings["tlsSettings"].toObject();
            profile.setSni(tlsSettings["serverName"].toString().toStdString());
            profile.setAllowInsecure(tlsSettings["allowInsecure"].toBool(false));

            QJsonArray alpn = tlsSettings["alpn"].toArray();
            if (alpn.size() > 0) {
                profile.setAlpn(alpn[0].toString().toStdString());
            }

            QString fingerprint = tlsSettings["fingerprint"].toString();
            if (!fingerprint.isEmpty()) {
                profile.setFingerprint(fingerprint.toStdString());
            }
        }

        if (security == "reality") {
            QJsonObject realitySettings = streamSettings["realitySettings"].toObject();
            profile.setSni(realitySettings["serverName"].toString().toStdString());

            QString fingerprint = realitySettings["fingerprint"].toString();
            if (!fingerprint.isEmpty()) {
                profile.setFingerprint(fingerprint.toStdString());
            }

            QString publicKey = realitySettings["publicKey"].toString();
            if (!publicKey.isEmpty()) {
                profile.setPublicKey(publicKey.toStdString());
            }

            QString shortId = realitySettings["shortId"].toString();
            if (!shortId.isEmpty()) {
                profile.setShortId(shortId.toStdString());
            }

            QString spiderX = realitySettings["spiderX"].toString();
            if (!spiderX.isEmpty()) {
                profile.setSpiderX(spiderX.toStdString());
            }
        }

        if (network == "ws") {
            QJsonObject wsSettings = streamSettings["wsSettings"].toObject();
            (void)wsSettings;
        } else if (network == "grpc") {
            QJsonObject grpcSettings = streamSettings["grpcSettings"].toObject();
            (void)grpcSettings;
        } else if (network == "h2") {
            QJsonObject httpSettings = streamSettings["httpSettings"].toObject();
            (void)httpSettings;
        }

        if (profile.isValid()) {
            tmp.push_back(profile);
        } else {
            qDebug() << "[DEBUG] Skipping invalid profile at index" << i
                     << "addr:" << QString::fromStdString(profile.getAddress()) << "port:" << profile.getPort()
                     << "remark:" << QString::fromStdString(profile.getRemark());
        }
    }

    out = std::move(tmp);
    qDebug() << "[DEBUG] Loaded" << out.size() << "valid servers from config.json";
    return true;
}

bool XrayConfigStore::saveServerProfiles(const QString& configFilePath,
                                         const std::vector<ProfileItem>& profiles, QString* errorMessage)
{
    QJsonObject root;
    QFile readFile(configFilePath);
    if (readFile.open(QIODevice::ReadOnly)) {
        QJsonDocument existingDoc = QJsonDocument::fromJson(readFile.readAll());
        readFile.close();
        if (existingDoc.isObject()) {
            root = existingDoc.object();
        }
    }

    QJsonArray outbounds;

    for (int i = 0; i < static_cast<int>(profiles.size()); ++i) {
        const ProfileItem& profile = profiles[static_cast<size_t>(i)];

        QJsonObject outbound;
        outbound["tag"] = QString::fromStdString(
            profile.getRemark().empty() ? QString("proxy-%1").arg(i + 1).toStdString() : profile.getRemark());

        QString protocol;
        switch (profile.getConfigType()) {
            case EConfigType::VMess:
                protocol = "vmess";
                break;
            case EConfigType::VLESS:
                protocol = "vless";
                break;
            case EConfigType::Trojan:
                protocol = "trojan";
                break;
            case EConfigType::Shadowsocks:
                protocol = "shadowsocks";
                break;
            default:
                protocol = "trojan";
        }
        outbound["protocol"] = protocol;

        QJsonObject settings;

        if (protocol == "trojan") {
            QJsonArray servers;
            QJsonObject server;
            server["address"] = QString::fromStdString(profile.getAddress());
            server["port"] = profile.getPort();
            server["password"] = QString::fromStdString(profile.getPassword());

            QString flow = QString::fromStdString(profile.getFlow());
            if (!flow.isEmpty()) {
                server["flow"] = flow;
            }

            server["ota"] = false;
            server["level"] = 1;
            servers.append(server);
            settings["servers"] = servers;
        } else if (protocol == "vmess") {
            QJsonArray vnext;
            QJsonObject server;
            server["address"] = QString::fromStdString(profile.getAddress());
            server["port"] = profile.getPort();

            QJsonArray users;
            QJsonObject user;
            user["id"] = QString::fromStdString(profile.getUserId());
            user["alterId"] = 0;
            user["security"] =
                QString::fromStdString(profile.getSecurity().empty() ? "auto" : profile.getSecurity());
            user["email"] = "user@v2raycpp";
            users.append(user);
            server["users"] = users;

            vnext.append(server);
            settings["vnext"] = vnext;
        } else if (protocol == "vless") {
            QJsonArray vnext;
            QJsonObject server;
            server["address"] = QString::fromStdString(profile.getAddress());
            server["port"] = profile.getPort();

            QJsonArray users;
            QJsonObject user;
            user["id"] = QString::fromStdString(profile.getUserId());
            user["email"] = "user@v2raycpp";
            user["encryption"] = "none";

            QString flow = QString::fromStdString(profile.getFlow());
            if (!flow.isEmpty()) {
                user["flow"] = flow;
            }

            users.append(user);
            server["users"] = users;

            vnext.append(server);
            settings["vnext"] = vnext;
        }

        outbound["settings"] = settings;

        QJsonObject streamSettings;
        QString network = QString::fromStdString(profile.getNetwork().empty() ? "tcp" : profile.getNetwork());
        streamSettings["network"] = network;

        QString security = QString::fromStdString(profile.getSecurity());
        if (!security.isEmpty() && security != "none") {
            streamSettings["security"] = security;

            if (security == "tls") {
                QJsonObject tlsSettings;
                QString sni = QString::fromStdString(profile.getSni());
                if (!sni.isEmpty()) {
                    tlsSettings["serverName"] = sni;
                }
                tlsSettings["allowInsecure"] = profile.getAllowInsecure();

                QString alpn = QString::fromStdString(profile.getAlpn());
                if (!alpn.isEmpty()) {
                    QJsonArray alpnArray;
                    alpnArray.append(alpn);
                    tlsSettings["alpn"] = alpnArray;
                }

                QString fingerprint = QString::fromStdString(profile.getFingerprint());
                if (!fingerprint.isEmpty()) {
                    tlsSettings["fingerprint"] = fingerprint;
                } else {
                    tlsSettings["fingerprint"] = "random";
                }

                streamSettings["tlsSettings"] = tlsSettings;
            } else if (security == "reality") {
                QJsonObject realitySettings;
                QString sni = QString::fromStdString(profile.getSni());
                if (!sni.isEmpty()) {
                    realitySettings["serverName"] = sni;
                }

                QString fingerprint = QString::fromStdString(profile.getFingerprint());
                if (!fingerprint.isEmpty()) {
                    realitySettings["fingerprint"] = fingerprint;
                }

                QString publicKey = QString::fromStdString(profile.getPublicKey());
                if (!publicKey.isEmpty()) {
                    realitySettings["publicKey"] = publicKey;
                }

                QString shortId = QString::fromStdString(profile.getShortId());
                if (!shortId.isEmpty()) {
                    realitySettings["shortId"] = shortId;
                }

                QString spiderX = QString::fromStdString(profile.getSpiderX());
                if (!spiderX.isEmpty()) {
                    realitySettings["spiderX"] = spiderX;
                }

                streamSettings["realitySettings"] = realitySettings;
            }
        }

        outbound["streamSettings"] = streamSettings;

        QJsonObject mux;
        mux["enabled"] = false;
        mux["concurrency"] = -1;
        outbound["mux"] = mux;

        outbounds.append(outbound);
    }

    bool hasDirect = false;
    bool hasBlock = false;
    for (const QJsonValue& ob : outbounds) {
        QString tag = ob.toObject()["tag"].toString();
        if (tag == "direct") {
            hasDirect = true;
        }
        if (tag == "block") {
            hasBlock = true;
        }
    }

    if (!hasDirect) {
        QJsonObject directOutbound;
        directOutbound["tag"] = "direct";
        directOutbound["protocol"] = "freedom";
        directOutbound["settings"] = QJsonObject();
        outbounds.append(directOutbound);
    }

    if (!hasBlock) {
        QJsonObject blockOutbound;
        blockOutbound["tag"] = "block";
        blockOutbound["protocol"] = "blackhole";
        blockOutbound["settings"] = QJsonObject();
        outbounds.append(blockOutbound);
    }

    root["outbounds"] = outbounds;

    if (!root.contains("log")) {
        QJsonObject log;
        log["loglevel"] = "warning";
        root["log"] = log;
    }

    if (!root.contains("inbounds")) {
        QJsonArray inbounds;
        QJsonObject inbound;
        inbound["tag"] = "socks-inbound";
        inbound["protocol"] = "socks";
        inbound["listen"] = "127.0.0.1";
        inbound["port"] = AppConfig::instance().getLocalPort();

        QJsonObject socksSettings;
        socksSettings["auth"] = "noauth";
        socksSettings["udp"] = true;
        inbound["settings"] = socksSettings;

        inbounds.append(inbound);
        root["inbounds"] = inbounds;
    }

    if (!root.contains("routing")) {
        QJsonObject routing;
        routing["domainStrategy"] = "IPIfNonMatch";
        routing["mode"] = "proxy";

        QJsonArray rules;
        QJsonObject rule;
        rule["type"] = "field";
        rule["outboundTag"] = "direct";

        QJsonArray domain;
        domain.append("geosite:cn");
        rule["domain"] = domain;

        QJsonArray ip;
        ip.append("geoip:private");
        ip.append("geoip:cn");
        rule["ip"] = ip;

        rules.append(rule);
        routing["rules"] = rules;

        root["routing"] = routing;
    }

    QJsonDocument doc(root);
    QFile file(configFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open config.json for writing");
        }
        qWarning() << "Failed to open config.json for writing:" << configFilePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Saved" << profiles.size() << "servers to config.json";
    return true;
}
