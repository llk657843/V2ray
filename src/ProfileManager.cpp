#include "ProfileManager.h"
#include "TrojanFmt.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

ProfileManager::ProfileManager()
{
    loadProfiles();
}

void ProfileManager::loadProfiles()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString serversFile = configPath + "/servers.json";

    QFile file(serversFile);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "No servers.json found, starting with empty list";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject())
    {
        qWarning() << "Invalid servers.json format";
        return;
    }

    m_serverProfiles.clear();
    QJsonObject root = doc.object();
    QJsonArray servers = root["servers"].toArray();

    for (int i = 0; i < servers.size(); ++i)
    {
        QJsonObject serverObj = servers[i].toObject();
        ProfileItem profile;
        profile.setAddress(serverObj["address"].toString().toStdString());
        profile.setPort(serverObj["port"].toInt());
        profile.setRemark(serverObj["remark"].toString().toStdString());
        profile.setPassword(serverObj["password"].toString().toStdString());
        profile.setSni(serverObj["sni"].toString().toStdString());
        profile.setNetwork(serverObj["network"].toString().toStdString());
        profile.setSecurity(serverObj["security"].toString().toStdString());
        profile.setFingerprint(serverObj["fingerprint"].toString().toStdString());
        profile.setAllowInsecure(serverObj["allowInsecure"].toBool());
        profile.setUserId(serverObj["userId"].toString().toStdString());
        profile.setAlterId(serverObj["alterId"].toString().toStdString());

        QString configTypeStr = serverObj["configType"].toString();
        if (configTypeStr == "VMess") profile.setConfigType(EConfigType::VMess);
        else if (configTypeStr == "VLESS") profile.setConfigType(EConfigType::VLESS);
        else if (configTypeStr == "Trojan") profile.setConfigType(EConfigType::Trojan);
        else if (configTypeStr == "Shadowsocks") profile.setConfigType(EConfigType::Shadowsocks);
        else profile.setConfigType(EConfigType::Trojan);

        if (profile.isValid())
        {
            m_serverProfiles.push_back(profile);
        }
    }
}

void ProfileManager::saveProfiles()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(configPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString serversFile = configPath + "/servers.json";

    QJsonObject root;
    QJsonArray servers;

    for (std::vector<ProfileItem>::const_iterator it = m_serverProfiles.begin(); it != m_serverProfiles.end(); ++it)
    {
        const ProfileItem& profile = *it;
        QJsonObject serverObj;
        serverObj["address"] = QString::fromStdString(profile.getAddress());
        serverObj["port"] = profile.getPort();
        serverObj["remark"] = QString::fromStdString(profile.getRemark());
        serverObj["password"] = QString::fromStdString(profile.getPassword());
        serverObj["sni"] = QString::fromStdString(profile.getSni());
        serverObj["network"] = QString::fromStdString(profile.getNetwork());
        serverObj["security"] = QString::fromStdString(profile.getSecurity());
        serverObj["fingerprint"] = QString::fromStdString(profile.getFingerprint());
        serverObj["allowInsecure"] = profile.getAllowInsecure();
        serverObj["userId"] = QString::fromStdString(profile.getUserId());
        serverObj["alterId"] = QString::fromStdString(profile.getAlterId());
        serverObj["configType"] = QString::fromStdString(profile.getConfigTypeString());
        servers.append(serverObj);
    }

    root["servers"] = servers;

    QFile file(serversFile);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open servers.json for writing:" << serversFile;
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
}

const std::vector<ProfileItem>& ProfileManager::getProfiles() const
{
    return m_serverProfiles;
}

std::vector<ProfileItem>& ProfileManager::getProfiles()
{
    return m_serverProfiles;
}

void ProfileManager::addProfile(const ProfileItem& profile)
{
    m_serverProfiles.push_back(profile);
}

bool ProfileManager::parseProfileFromUrl(const QString& url, ProfileItem& profile)
{
    if (url.startsWith("trojan://"))
    {
        // 使用 std::unique_ptr 替换已废弃的 std::auto_ptr
        std::unique_ptr<ProfileItem> parsed = TrojanFmt::parse(url.toStdString());
        if (parsed)
        {
            profile = *parsed;
            return true;
        }
    }
    // Placeholder for other protocols
    return false;
}
