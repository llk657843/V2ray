#include "Logger.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

// ============ Static Members ============
Logger* Logger::s_instance = nullptr;
QMutex Logger::s_mutex;

// ============ Constructor ============
Logger::Logger(QObject* parent)
    : QObject(parent)
    , m_level(LOG_INFO)
    , m_fileLogEnabled(true)
    , m_consoleLogEnabled(true)
{
    // Set default log directory: %APPDATA%\v2raycpp\logs
    m_logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_logDir += "/logs/";
    
    // Create log directory if not exists
    QDir dir(m_logDir);
    if (!dir.exists())
    {
        dir.mkpath(m_logDir);
    }
    
    // Open log file
    openLogFile();
}

Logger::~Logger()
{
    closeLogFile();
}

// ============ Singleton ============
Logger* Logger::instance(QObject* parent)
{
    if (s_instance == nullptr)
    {
        QMutexLocker locker(&s_mutex);
        if (s_instance == nullptr)
        {
            s_instance = new Logger(parent);
        }
    }
    return s_instance;
}

// ============ Private Methods ============
QString Logger::levelToString(LogLevel level) const
{
    switch (level)
    {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR:   return "ERROR";
        default:      return "UNKNOWN";
    }
}

QString Logger::getLogFileName() const
{
    return m_logDir + "/v2raycpp_" + QDate::currentDate().toString("yyyyMMdd") + ".log";
}

void Logger::openLogFile()
{
    m_currentLogFile = getLogFileName();
    m_logFile.setFileName(m_currentLogFile);
    
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        m_logStream.setDevice(&m_logFile);
    }
}

void Logger::closeLogFile()
{
    if (m_logFile.isOpen())
    {
        m_logStream.flush();
        m_logFile.close();
    }
}

void Logger::checkDateChange()
{
    QString newLogFile = getLogFileName();
    if (newLogFile != m_currentLogFile)
    {
        closeLogFile();
        openLogFile();
    }
}

// ============ Public Log Methods ============
void Logger::log(LogLevel level, const QString& message)
{
    // Check if level is sufficient
    if (level < m_level)
    {
        return;
    }

    // Check date change for log rotation
    if (m_fileLogEnabled)
    {
        checkDateChange();
    }

    // Format: YYYY-MM-DD HH:mm:ss LEVEL Message
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString levelStr = levelToString(level);
    QString formattedMessage = QString("%1 - %2 - %3").arg(timestamp, levelStr, message);

    // Console output
    if (m_consoleLogEnabled)
    {
        if (level >= LOG_WARNING)
        {
            qWarning() << formattedMessage;
        }
        else
        {
            qDebug() << formattedMessage;
        }
    }

    // File output
    if (m_fileLogEnabled && m_logFile.isOpen())
    {
        QMutexLocker locker(&m_fileMutex);
        m_logStream << formattedMessage << Qt::endl;
        m_logStream.flush();
    }

    // Emit signal for UI
    emit logMessage(formattedMessage);
}

void Logger::debug(const QString& message)
{
    log(LOG_DEBUG, message);
}

void Logger::info(const QString& message)
{
    log(LOG_INFO, message);
}

void Logger::warning(const QString& message)
{
    log(LOG_WARNING, message);
}

void Logger::error(const QString& message)
{
    log(LOG_ERROR, message);
}

// ============ Configuration ============
void Logger::setLevel(LogLevel level)
{
    m_level = level;
}

void Logger::enableFileLog(bool enabled)
{
    m_fileLogEnabled = enabled;
    if (enabled && !m_logFile.isOpen())
    {
        openLogFile();
    }
}

void Logger::enableConsoleLog(bool enabled)
{
    m_consoleLogEnabled = enabled;
}
