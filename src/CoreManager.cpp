#include "CoreManager.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>
#include <QThread>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

// ============ CoreManager Implementation ============

CoreManager::CoreManager(QObject* parent)
    : QObject(parent)
{
    // Initialize with default core path
    m_coreExePath = AppConfig::instance().getDefaultCorePath();
    
    // Initialize log file
    QString logPath = AppConfig::instance().getLogFilePath();
    if (logPath.isEmpty())
    {
        // Default log path
        logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(logPath);
        if (!dir.exists())
        {
            dir.mkpath(logPath);
        }
        logPath += "/v2raycpp.log";
    }
    
    m_logFile.setFileName(logPath);
}

CoreManager::~CoreManager()
{
    // Ensure process is stopped
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        stopCore();
    }
}

QString CoreManager::getCoreFileName() const
{
#ifdef Q_OS_WIN
    return "xray.exe";
#else
    return "xray";
#endif
}

bool CoreManager::checkCoreExists()
{
    QString corePath = getCoreExePath();
    QFileInfo fileInfo(corePath);
    
    if (!fileInfo.exists())
    {
        emit errorOutput(QString("Core executable not found: %1").arg(corePath));
        return false;
    }
    
    if (!fileInfo.isExecutable())
    {
        emit errorOutput(QString("Core executable is not executable: %1").arg(corePath));
        return false;
    }
    
    return true;
}

bool CoreManager::startCore(const QString& configFile)
{
    // Check if already running
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        emit logOutput("Core is already running");
        return false;
    }
    
    // Check core exists
    if (!checkCoreExists())
    {
        return false;
    }
    
    // Check config file exists
    QFileInfo configInfo(configFile);
    if (!configInfo.exists())
    {
        emit errorOutput(QString("Config file not found: %1").arg(configFile));
        return false;
    }
    
    // Set status to starting
    setStatus(CoreStatus::Starting);
    
    // Create new process
    m_process = std::make_unique<QProcess>();
    
    // Connect signals
    connect(m_process.get(), &QProcess::readyReadStandardOutput,
            this, &CoreManager::onReadyReadStandardOutput);
    connect(m_process.get(), &QProcess::readyReadStandardError,
            this, &CoreManager::onReadyReadStandardError);
    connect(m_process.get(), &QProcess::stateChanged,
            this, &CoreManager::onProcessStateChanged);
    connect(m_process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CoreManager::onProcessFinished);
    
    // Set working directory to core executable directory
    QFileInfo coreInfo(m_coreExePath);
    m_process->setWorkingDirectory(coreInfo.absolutePath());
    
    // Build arguments
    QStringList arguments;
    arguments << "-config" << configFile;
    
    // Store current config file
    m_currentConfigFile = configFile;
    
    // Start process
    emit logOutput(QString("Starting core: %1").arg(m_coreExePath));
    emit logOutput(QString("Config: %1").arg(configFile));
    
    m_process->start(m_coreExePath, arguments);
    
    // Wait for process to start (non-blocking check)
    if (!m_process->waitForStarted(3000))
    {
        setStatus(CoreStatus::Stopped);
        emit errorOutput(QString("Failed to start core: %1").arg(m_process->errorString()));
        return false;
    }
    
    return true;
}

bool CoreManager::stopCore()
{
    if (!m_process || m_process->state() == QProcess::NotRunning)
    {
        emit logOutput("Core is not running");
        return true;
    }
    
    setStatus(CoreStatus::Stopping);
    
    // Try graceful termination first
    m_process->terminate();
    
    // Wait for graceful termination
    if (!m_process->waitForFinished(3000))
    {
        // Force kill if graceful termination fails
        emit logOutput("Core did not respond to terminate, forcing kill");
        m_process->kill();
        m_process->waitForFinished(1000);
    }
    
    setStatus(CoreStatus::Stopped);
    m_process.reset();
    
    emit logOutput("Core stopped");
    return true;
}

CoreStatus CoreManager::getStatus() const
{
    return m_status;
}

bool CoreManager::isRunning() const
{
    return m_status == CoreStatus::Running;
}

bool CoreManager::reloadConfig()
{
    if (!m_process || m_process->state() != QProcess::Running)
    {
        emit errorOutput("Core is not running, cannot reload config");
        return false;
    }
    
    // Xray supports config hot reload via API
    // For now, we restart with the same config
    QString configFile = m_currentConfigFile;
    
    if (configFile.isEmpty())
    {
        emit errorOutput("No config file to reload");
        return false;
    }
    
    emit logOutput("Reloading config...");
    
    // Stop current process
    stopCore();
    
    // Small delay
    QThread::msleep(100);
    
    // Restart with new config
    return startCore(configFile);
}

QString CoreManager::getCoreExePath() const
{
    return m_coreExePath;
}

void CoreManager::setCoreExePath(const QString& path)
{
    m_coreExePath = path;
    AppConfig::instance().setCorePath(path);
}

// ============ Xray API Implementation ============

QString CoreManager::getXrayApiPath() const
{
    return m_xrayApiPath;
}

void CoreManager::setXrayApiPath(const QString& path)
{
    m_xrayApiPath = path;
}

QString CoreManager::callXrayApi(const QString& command)
{
    QString apiPath = m_xrayApiPath;
    if (apiPath.isEmpty())
    {
        // Default to xray-api.exe in the same directory as xray.exe
        QFileInfo coreInfo(m_coreExePath);
        apiPath = coreInfo.absolutePath() + "/xray-api.exe";
    }

    QProcess process;
    process.start(apiPath, QStringList() << "-cmd" << command);
    
    if (!process.waitForFinished(5000))
    {
        return QString("{\"error\": \"timeout\"}");
    }
    
    QString output = process.readAllStandardOutput();
    return output;
}

QStringList CoreManager::listInbounds()
{
    QString result = callXrayApi("list_inbounds");
    QStringList tags;
    
    // Parse JSON response to extract inbound tags
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    if (doc.isObject() && doc.object().contains("inbounds"))
    {
        QJsonArray inbounds = doc.object()["inbounds"].toArray();
        for (const QJsonValue& ib : inbounds)
        {
            tags.append(ib.toObject()["tag"].toString());
        }
    }
    
    return tags;
}

QStringList CoreManager::listOutbounds()
{
    QString result = callXrayApi("list_outbounds");
    QStringList tags;
    
    // Parse JSON response to extract outbound tags
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    if (doc.isObject() && doc.object().contains("outbounds"))
    {
        QJsonArray outbounds = doc.object()["outbounds"].toArray();
        for (const QJsonValue& ob : outbounds)
        {
            tags.append(ob.toObject()["tag"].toString());
        }
    }
    
    return tags;
}

bool CoreManager::removeInbound(const QString& tag)
{
    QProcess process;
    QString apiPath = m_xrayApiPath;
    if (apiPath.isEmpty())
    {
        QFileInfo coreInfo(m_coreExePath);
        apiPath = coreInfo.absolutePath() + "/xray-api.exe";
    }
    
    process.start(apiPath, QStringList() << "-cmd" << "remove_inbound" << "-tag" << tag);
    
    if (!process.waitForFinished(5000))
    {
        return false;
    }
    
    return process.exitCode() == 0;
}

bool CoreManager::removeOutbound(const QString& tag)
{
    QProcess process;
    QString apiPath = m_xrayApiPath;
    if (apiPath.isEmpty())
    {
        QFileInfo coreInfo(m_coreExePath);
        apiPath = coreInfo.absolutePath() + "/xray-api.exe";
    }
    
    process.start(apiPath, QStringList() << "-cmd" << "remove_outbound" << "-tag" << tag);
    
    if (!process.waitForFinished(5000))
    {
        return false;
    }
    
    return process.exitCode() == 0;
}

// ============ Private Slots ============

void CoreManager::onReadyReadStandardOutput()
{
    if (!m_process) return;
    
    QByteArray data = m_process->readAllStandardOutput();
    QString output = QString::fromUtf8(data).trimmed();
    
    if (!output.isEmpty() && m_displayLog)
    {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString logLine = QString("[%1] [stdout] %2").arg(timestamp, output);
        
        emit logOutput(logLine);
        writeLogToFile(logLine);
    }
}

void CoreManager::onReadyReadStandardError()
{
    if (!m_process) return;
    
    QByteArray data = m_process->readAllStandardError();
    QString output = QString::fromUtf8(data).trimmed();
    
    if (!output.isEmpty() && m_displayLog)
    {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString logLine = QString("[%1] [stderr] %2").arg(timestamp, output);
        
        emit errorOutput(logLine);
        writeLogToFile(logLine);
    }
}

void CoreManager::onProcessStateChanged(QProcess::ProcessState state)
{
    switch (state)
    {
        case QProcess::NotRunning:
            setStatus(CoreStatus::Stopped);
            break;
        case QProcess::Starting:
            setStatus(CoreStatus::Starting);
            break;
        case QProcess::Running:
            setStatus(CoreStatus::Running);
            emit logOutput("Core is running");
            break;
    }
}

void CoreManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString message = QString("Core process finished. Exit code: %1, Status: %2")
        .arg(exitCode)
        .arg(exitStatus == QProcess::NormalExit ? "Normal" : "Crash");
    
    emit logOutput(message);
    emit processFinished(exitCode, exitStatus);
    
    setStatus(CoreStatus::Stopped);
}

// ============ Private Methods ============

void CoreManager::writeLogToFile(const QString& log)
{
    if (!AppConfig::instance().isLogEnabled())
    {
        return;
    }
    
    if (m_logFile.isOpen())
    {
        QTextStream stream(&m_logFile);
        stream << log << "\n";
        stream.flush();
    }
    else if (m_logFile.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream stream(&m_logFile);
        stream << log << "\n";
        stream.flush();
        m_logFile.close();
    }
}

void CoreManager::setStatus(CoreStatus status)
{
    if (m_status != status)
    {
        m_status = status;
        emit statusChanged(status);
    }
}

