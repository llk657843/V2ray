#include "v2raycpp.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFont>

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

    // Set font for Chinese support
    QFont font("Microsoft YaHei", 9);
    app.setFont(font);

    v2raycpp window;
    window.show();
    return app.exec();
}
