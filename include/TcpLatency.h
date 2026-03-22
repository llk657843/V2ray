#pragma once

#include <QString>

/// TCP 连接耗时（毫秒），失败返回 -1
namespace TcpLatency {
int measureConnectMs(const QString& host, int port, int timeoutMs = 3000);
}
