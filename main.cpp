#include "v2raycpp.h"
#include "login\LoginMainWidget.h"
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

    // Load all .qss files from style/ directory (sorted alphabetically, main.qss first)
    QString styleDirPath = QCoreApplication::applicationDirPath() + "/../../style/";
    QDir styleDir(styleDirPath);
    QStringList qssFiles = styleDir.entryList(QStringList() << "*.qss", QDir::Files);
    qssFiles.sort(Qt::CaseInsensitive);

    QString combinedStyleSheet;
    for (const QString &qssFile : qssFiles) {
        QFile file(styleDir.filePath(qssFile));
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            combinedStyleSheet += stream.readAll();
            file.close();
        }
    }
    if (!combinedStyleSheet.isEmpty()) {
        app.setStyleSheet(combinedStyleSheet);
    }


    // Create login widget and show it first
    LoginMainWidget LoginMainWidget;
    LoginMainWidget.show();

    // When login succeeds, hide login and show main window
    QObject::connect(&LoginMainWidget, &LoginMainWidget::loginSuccess, [&]() {
        LoginMainWidget.hide();
    });

    // When login is closed (user clicks X), exit app
    QObject::connect(&LoginMainWidget, &LoginMainWidget::loginClose, [&]() {
        QApplication::quit();
    });

    return app.exec();
}
