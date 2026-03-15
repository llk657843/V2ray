#pragma once

#include <QtWidgets/QMainWindow>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <memory>
#include <vector>
#include "ui_v2raycpp.h"

#include "CoreManager.h"
#include "SysProxyHandler.h"
#include "AppConfig.h"
#include "ProfileItem.h"
#include "TrayIcon.h"
#include "ServerGridWidget.h"

class v2raycpp : public QWidget
{
    Q_OBJECT

public:
    v2raycpp(QWidget* parent = nullptr);
    ~v2raycpp();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    //void onCloseClicked();
    void onStartClicked();
    void onStopClicked();
    void onImportClicked();
    void onAddServerClicked();
    void onSettingsClicked();
    void onServerDoubleClicked();
    void onServerSelected(int currentRow);
    void onEditServerClicked();
    void onDeleteServerClicked();
    void onLogOutput(const QString& log);
    void onErrorOutput(const QString& error);
    void onStatusChanged(CoreStatus status);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onShowWindowFromTray();
    void onSettingsFromTray();
    void onExitFromTray();
    void onTrayDoubleClicked();
    void onEnableSystemProxy();
    void onDisableSystemProxy();
    void onRefreshLatencyClicked();
    void onCustomContextMenu(const QPoint& pos);
    void onSearchTextChanged(const QString& text);

private:
    void initUI();
    void initConnections();
    void updateUIStatus();
    bool generateCoreConfig(const ProfileItem& profile);
    void loadConfig();
    void saveConfig();
    void addServerToList(const ProfileItem& profile);
    ProfileItem getSelectedProfile() const;
    bool parseProfileFromUrl(const QString& url, ProfileItem& profile);
    void updateStatusBar();
    void loadStyleSheet();
    void initServerGrid();
    void testLatency(const QString& address, int port);
    // 新增：把 card 与服务器索引关联，默认 -1 表示没有绑定 profile
    void addCardToGrid(const QString& title, const QString& protocol = QString(), int latency = -1, bool connected = false, int serverIndex = -1);

    // Traffic statistics
    qint64 m_bytesReceived = 0;
    qint64 m_bytesSent = 0;
    qint64 m_lastBytesReceived = 0;
    qint64 m_lastBytesSent = 0;
    QTimer* m_statsTimer = nullptr;
    void startStatsTimer();
    void stopStatsTimer();
    void updateStats();
    void startReconnectTimer();
    void stopReconnectTimer();
    void onReconnectTimeout();
    // Auto reconnect
    QTimer* m_reconnectTimer = nullptr;
    bool m_autoReconnect = true;


private:
    Ui::v2raycppClass ui;
    std::unique_ptr<CoreManager> m_coreManager;
    std::unique_ptr<SysProxyHandler> m_sysProxyHandler;
    ProfileItem m_currentProfile;
    std::vector<ProfileItem> m_serverProfiles;
    CoreStatus m_currentStatus = CoreStatus::Stopped;
    std::unique_ptr<TrayIcon> m_trayIcon;
    QDateTime m_startTime;

    // For window dragging
    bool m_dragging;
    QPoint m_dragPosition;

    // Server grid (new card-style UI)
    ServerGridWidget* m_serverGrid = nullptr;
};