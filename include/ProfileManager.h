#pragma once

#include <vector>
#include <QString>
#include "ProfileItem.h"

class ProfileManager
{
public:
    ProfileManager();
    ~ProfileManager() = default;

    void loadProfiles();
    void saveProfiles();

    const std::vector<ProfileItem>& getProfiles() const;
    std::vector<ProfileItem>& getProfiles();
    void addProfile(const ProfileItem& profile);

    static bool parseProfileFromUrl(const QString& url, ProfileItem& profile);

private:
    std::vector<ProfileItem> m_serverProfiles;
};
