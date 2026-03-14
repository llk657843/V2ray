#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <memory>

#include "AppConfig.h"

/// <summary>
/// Core process status enum
/// </summary>
enum class CoreStatus
{
    Stopped,    // Stopped
    Running,    // Running
    Starting,   // Starting
    Stopping    // Stopping
};

/// <summary>
/// Core process management class
/// Responsible for managing Xray-core process start, stop, status detection and log capture
/// </summary>
class CoreManager : public QObject
{
    Q_OBJECT

public:
    /// <summary>
    /// Get singleton instance
    /// </summary>
    static CoreManager& instance()
    {
        static CoreManager manager;
        return manager;
    }

    /// <summary>
    /// Initialize CoreManager
    /// </summary>
    /// <param name="parent">Parent object</param>
    explicit CoreManager(QObject* parent = nullptr);
    ~CoreManager();

    // ============ Core Functions ============

    /// <summary>
    /// Check if Xray-core exists
    /// </summary>
    /// <returns>true if core executable exists</returns>
    bool checkCoreExists();

    /// <summary>
    /// Start Xray-core process
    /// </summary>
    /// <param name="configFile">Config file path</param>
    /// <returns>true if start successful</returns>
    bool startCore(const QString& configFile);

    /// <summary>
    /// Stop Xray-core process
    /// </summary>
    /// <returns>true if stop successful</returns>
    bool stopCore();

    /// <summary>
    /// Get current process status
    /// </summary>
    /// <returns>Process status</returns>
    CoreStatus getStatus() const;

    /// <summary>
    /// Check if process is running
    /// </summary>
    /// <returns>true if process is running</returns>
    bool isRunning() const;

    /// <summary>
    /// Reload config file (hot reload)
    /// Send signal to core to reload config
    /// </summary>
    /// <returns>true if reload signal sent successfully</returns>
    bool reloadConfig();

    /// <summary>
    /// Get core executable file path
    /// </summary>
    /// <returns>Core executable full path</returns>
    QString getCoreExePath() const;

    /// <summary>
    /// Set core executable file path
    /// </summary>
    /// <param name="path">Core executable path</param>
    void setCoreExePath(const QString& path);

    // ============ Xray API Functions ============

    /// <summary>
    /// Get xray-api executable path
    /// </summary>
    QString getXrayApiPath() const;

    /// <summary>
    /// Set xray-api executable path
    /// </summary>
    void setXrayApiPath(const QString& path);

    /// <summary>
    /// Call xray API via gRPC
    /// </summary>
    /// <param name="command">API command (list_inbounds, list_outbounds, etc.)</param>
    /// <returns>JSON response string</returns>
    QString callXrayApi(const QString& command);

    /// <summary>
    /// List all inbound handlers
    /// </summary>
    QStringList listInbounds();

    /// <summary>
    /// List all outbound handlers
    /// </summary>
    QStringList listOutbounds();

    /// <summary>
    /// Remove an inbound handler
    /// </summary>
    bool removeInbound(const QString& tag);

    /// <summary>
    /// Remove an outbound handler
    /// </summary>
    bool removeOutbound(const QString& tag);

signals:
    /// <summary>
    /// Log output signal
    /// </summary>
    /// <param name="log">Log content</param>
    void logOutput(const QString& log);

    /// <summary>
    /// Error output signal
    /// </summary>
    /// <param name="error">Error content</param>
    void errorOutput(const QString& error);

    /// <summary>
    /// Process status changed signal
    /// </summary>
    /// <param name="status">New status</param>
    void statusChanged(CoreStatus status);

    /// <summary>
    /// Process abnormal exit signal
    /// </summary>
    /// <param name="exitCode">Exit code</param>
    /// <param name="exitStatus">Exit status</param>
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private slots:
    /// <summary>
    /// Handle standard output data
    /// </summary>
    void onReadyReadStandardOutput();

    /// <summary>
    /// Handle standard error data
    /// </summary>
    void onReadyReadStandardError();

    /// <summary>
    /// Handle process state change
    /// </summary>
    void onProcessStateChanged(QProcess::ProcessState state);

    /// <summary>
    /// Handle process finished
    /// </summary>
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    /// <summary>
    /// Write log to file
    /// </summary>
    void writeLogToFile(const QString& log);

    /// <summary>
    /// Set process status
    /// </summary>
    void setStatus(CoreStatus status);

    /// <summary>
    /// Get core file name
    /// </summary>
    QString getCoreFileName() const;

private:
    std::unique_ptr<QProcess> m_process;    // Xray-core process
    CoreStatus m_status = CoreStatus::Stopped;  // Current status
    QString m_coreExePath;                   // Core executable path
    QString m_currentConfigFile;             // Current config file
    QString m_xrayApiPath;                   // xray-api executable path
    QFile m_logFile;                         // Log file
    bool m_displayLog = true;                // Whether to display log
};
