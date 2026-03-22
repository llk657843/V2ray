#pragma once

#include <QString>

/// 底部工具栏图标统一走 :/images/figma/icon_footer_*.png。
/// 若 Figma 导出「文件名与图案」不一致，只需改 v2raycpp.qrc 里 file alias 指向的物理文件，勿改此处函数名。
namespace FooterBarIcons {
inline QString connection()
{
    return QStringLiteral(":/images/figma/icon_footer_conn.png");
}
inline QString uploadSpeed()
{
    return QStringLiteral(":/images/figma/icon_footer_up.png");
}
inline QString downloadSpeed()
{
    return QStringLiteral(":/images/figma/icon_footer_down.png");
}
inline QString globeIp()
{
    return QStringLiteral(":/images/figma/icon_footer_globe.png");
}
inline QString clockUptime()
{
    return QStringLiteral(":/images/figma/icon_footer_clock.png");
}
} // namespace FooterBarIcons
