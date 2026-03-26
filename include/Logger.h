#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>

// Log level definitions
enum LogLevel
{
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
	LOG_ERROR = 3
};

class Logger : public QObject
{
    Q_OBJECT

private:
    static Logger* s_instance;
    static QMutex s_mutex;

    LogLevel m_level;
    bool m_fileLogEnabled;
    bool m_consoleLogEnabled;
    QString m_logDir;
    QString m_currentLogFile;
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_fileMutex;

    Logger(QObject* parent = nullptr);
    ~Logger();

    QString levelToString(LogLevel level) const;
    QString getLogFileName() const;
    void openLogFile();
    void closeLogFile();
    void checkDateChange();

public:
    static Logger* instance(QObject* parent = nullptr);

    // Disable copy
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Log methods
    void log(LogLevel level, const QString& message);
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);

    // Configuration
    void setLevel(LogLevel level);
    LogLevel getLevel() const { return m_level; }
    void enableFileLog(bool enabled);
    void enableConsoleLog(bool enabled);
    bool isFileLogEnabled() const { return m_fileLogEnabled; }
    bool isConsoleLogEnabled() const { return m_consoleLogEnabled; }

signals:
    // Signal to send log to UI
    void logMessage(QString message);
};

#endif // LOGGER_H
