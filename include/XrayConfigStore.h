#pragma once

#include "ProfileItem.h"
#include <QString>
#include <vector>

/// 读写 Xray 多节点 config.json（与 ConfigGenerator 生成的单节点启动配置不同）
class XrayConfigStore
{
public:
    /// 从 config.json 解析代理节点列表；成功时替换 \p out。文件不存在时保持 \p out 不变并返回 true。
    static bool loadServerProfiles(const QString& configFilePath, std::vector<ProfileItem>& out,
                                   QString* errorMessage = nullptr);

    /// 将节点列表写回 config.json，保留已有 log / inbounds / routing 等字段并替换 outbounds。
    static bool saveServerProfiles(const QString& configFilePath, const std::vector<ProfileItem>& profiles,
                                   QString* errorMessage = nullptr);
};
