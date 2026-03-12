#include "v2raycpp.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>

int main(int argc, char *argv[])
{
    // Create log file
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    logPath += "/logs/";
    QDir().mkpath(logPath);
    logPath += "v2raycpp_" + QDate::currentDate().toString("yyyyMMdd") + ".log";

    QFile logFile(logPath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&logFile);
        stream << "=== App Started ===" << Qt::endl;
        logFile.close();
    }

    QApplication app(argc, argv);
    v2raycpp window;
    window.show();
    return app.exec();
}
