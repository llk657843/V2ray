#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QPainter>
#include <QColor>

/// <summary>
/// Tray icon states
/// </summary>
enum class ETrayStatus
{
    Stopped,    // Gray - not connected
    Running,    // Green - running
    Disconnected, // Red - disconnected
};

/// <summary>
/// TrayIcon class - manages system tray icon and context menu
/// </summary>
class TrayIcon : public QObject
{
    Q_OBJECT

public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name=" parent\>Parent object</param>
 explicit TrayIcon(QObject* parent = nullptr);

 /// <summary>
 /// Destructor
 /// </summary>
 ~TrayIcon();

 /// <summary>
 /// Initialize tray icon and menu
 /// </summary>
 void init();

 /// <summary>
 /// Set tray icon status
 /// </summary>
 /// <param name=\status\>New status</param>
 void setStatus(ETrayStatus status);

 /// <summary>
 /// Show the tray icon
 /// </summary>
 void show();

 /// <summary>
 /// Hide the tray icon
 /// </summary>
 void hide();

signals:
 /// <summary>
 /// Signal emitted when \Start Proxy\ is clicked
 /// </summary>
 void startProxyClicked();

 /// <summary>
 /// Signal emitted when \Stop Proxy\ is clicked
 /// </summary>
 void stopProxyClicked();

 /// <summary>
 /// Signal emitted when \Enable System Proxy\ is clicked
 /// </summary>
 void enableSystemProxyClicked();

 /// <summary>
 /// Signal emitted when \Disable System Proxy\ is clicked
 /// </summary>
 void disableSystemProxyClicked();

 /// <summary>
 /// Signal emitted when \Show Window\ is clicked
 /// </summary>
 void showWindowClicked();

 /// <summary>
 /// Signal emitted when \Settings\ is clicked
 /// </summary>
 void settingsClicked();

 /// <summary>
 /// Signal emitted when \Exit\ is clicked
 /// </summary>
 void exitClicked();

 /// <summary>
 /// Signal emitted when tray icon is double-clicked
 /// </summary>
 void trayIconDoubleClicked();

private slots:
 /// <summary>
 /// Handle tray icon activation (double-click)
 /// </summary>
 void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

 /// <summary>
 /// Handle start proxy action
 /// </summary>
 void onStartProxy();

 /// <summary>
 /// Handle stop proxy action
 /// </summary>
 void onStopProxy();

 /// <summary>
 /// Handle enable system proxy action
 /// </summary>
 void onEnableSystemProxy();

 /// <summary>
 /// Handle disable system proxy action
 /// </summary>
 void onDisableSystemProxy();

 /// <summary>
 /// Handle show window action
 /// </summary>
 void onShowWindow();

 /// <summary>
 /// Handle settings action
 /// </summary>
 void onSettings();

 /// <summary>
 /// Handle exit action
 /// </summary>
 void onExit();

private:
 /// <summary>
 /// Create tray icon based on status
 /// </summary>
 QIcon createTrayIcon(ETrayStatus status);

 /// <summary>
 /// Update menu items based on current status
 /// </summary>
 void updateMenuState();

private:
 /// <summary>
 /// System tray icon
 /// </summary>
 QSystemTrayIcon* m_trayIcon;

 /// <summary>
 /// Context menu
 /// </summary>
 QMenu* m_menu;

 /// <summary>
 /// Current status
 /// </summary>
 ETrayStatus m_currentStatus;

 // Menu actions
 QAction* m_startProxyAction;
 QAction* m_stopProxyAction;
 QAction* m_enableSystemProxyAction;
 QAction* m_disableSystemProxyAction;
 QAction* m_showWindowAction;
 QAction* m_settingsAction;
 QAction* m_exitAction;

 // System proxy state
 bool m_isSystemProxyEnabled;
};

