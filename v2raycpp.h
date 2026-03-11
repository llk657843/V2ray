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

class v2raycpp : public QMainWindow
{
    Q_OBJECT

public:
    v2raycpp(QWidget *parent = nullptr);
    ~v2raycpp();

private slots:
    void onStartClicked();
    void onStopClicked();
    void onImportClicked();
    void onAddServerClicked();
    void onSettingsClicked();
    void onServerDoubleClicked(const QModelIndex& index);
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

private:
    Ui::v2raycppClass ui;
    std::unique_ptr<CoreManager> m_coreManager;
    std::unique_ptr<SysProxyHandler> m_sysProxyHandler;
    ProfileItem m_currentProfile;
    std::vector<ProfileItem> m_serverProfiles;
    CoreStatus m_currentStatus = CoreStatus::Stopped;
    std::unique_ptr<TrayIcon> m_trayIcon;
    QDateTime m_startTime;
};