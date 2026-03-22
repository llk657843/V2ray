#pragma once

#include <QtWidgets/QMainWindow>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QPointer>
#include <QVector>
#include <memory>
#include <vector>
#include "ui_v2raycpp.h"

#include "CoreManager.h"
#include "SysProxyHandler.h"
#include "AppConfig.h"
#include "ProfileItem.h"
#include "TrayIcon.h"

class QButtonGroup;
class SimpleCard;
class ServerCardProbeService;
class TrafficStatsController;
class ReconnectController;

class v2raycpp : public QWidget
{
    Q_OBJECT

public:
    v2raycpp(QWidget* parent = nullptr);
    ~v2raycpp();

signals:
    void accountLogoutRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    //void onCloseClicked();
    void onStartClicked();
    void onStopClicked();
    void onDisconnectAccountClicked();
    void onImportClicked();
    void onSettingsClicked();
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
    void onSearchTextChanged(const QString& text);
    void onProbeLatency(int serverIndex, int latencyMs);
    void onProbeGeo(int serverIndex, const QString& primaryLine, const QString& secondaryLine,
                    const QString& country, const QString& countryCode);

private:
    void initUI();
    void initServerGrid();
    void initConnections();
    void updateUIStatus();
    void loadConfig();
    void saveConfig();
    void addServerToList(const ProfileItem& profile);
    ProfileItem getSelectedProfile() const;
    void updateStatusBar();
    void addCardToGrid(const QString& title, const QString& protocol = QString(), int latency = -1, bool connected = false, int serverIndex = -1);
    void startCardServerProbe(SimpleCard* card, int serverIndex);
    /// 按协议筛选 + 搜索框关键字显示/隐藏卡片
    void updateServerCardFilter();
    void applyServerCardSelection(int serverIndex);
    void syncServerCardSelectionWithCurrentProfile();
    void startStatsTimer();
    void stopStatsTimer();
    void startReconnectTimer();
    void stopReconnectTimer();
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
    ServerCardProbeService* m_serverProbe = nullptr;
    TrafficStatsController* m_trafficStats = nullptr;
    ReconnectController* m_reconnectController = nullptr;
    /// 与 m_serverProfiles 下标一一对应，用于刷新延迟等
    QVector<QPointer<SimpleCard>> m_serverCards;
    QButtonGroup* m_protocolFilterGroup = nullptr;

    // For window dragging
    bool m_dragging;
    QPoint m_dragPosition;
};