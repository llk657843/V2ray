/********************************************************************************
** Form generated from reading UI file 'v2raycpp.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_V2RAYCPP_H
#define UI_V2RAYCPP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_v2raycppClass
{
public:
    QAction *actionStart;
    QAction *actionStop;
    QAction *actionImport;
    QAction *actionAdd;
    QAction *actionSettings;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuServer;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QListWidget *serverList;
    QStatusBar *statusBar;
    QLabel *statusLabel;
    QLabel *currentNodeLabel;
    QLabel *startTimeLabel;

    void setupUi(QMainWindow *v2raycppClass)
    {
        if (v2raycppClass->objectName().isEmpty())
            v2raycppClass->setObjectName("v2raycppClass");
        v2raycppClass->resize(900, 600);
        v2raycppClass->setMinimumSize(QSize(600, 400));
        actionStart = new QAction(v2raycppClass);
        actionStart->setObjectName("actionStart");
        actionStop = new QAction(v2raycppClass);
        actionStop->setObjectName("actionStop");
        actionImport = new QAction(v2raycppClass);
        actionImport->setObjectName("actionImport");
        actionAdd = new QAction(v2raycppClass);
        actionAdd->setObjectName("actionAdd");
        actionSettings = new QAction(v2raycppClass);
        actionSettings->setObjectName("actionSettings");
        menuBar = new QMenuBar(v2raycppClass);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 900, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName("menuFile");
        menuServer = new QMenu(menuBar);
        menuServer->setObjectName("menuServer");
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName("menuHelp");
        v2raycppClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(v2raycppClass);
        mainToolBar->setObjectName("mainToolBar");
        mainToolBar->setMovable(false);
        mainToolBar->setIconSize(QSize(24, 24));
        v2raycppClass->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        centralWidget = new QWidget(v2raycppClass);
        centralWidget->setObjectName("centralWidget");
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName("verticalLayout");
        serverList = new QListWidget(centralWidget);
        serverList->setObjectName("serverList");
        serverList->setAlternatingRowColors(true);
        serverList->setSelectionMode(QAbstractItemView::SingleSelection);
        serverList->setSelectionBehavior(QAbstractItemView::SelectRows);

        verticalLayout->addWidget(serverList);

        v2raycppClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(v2raycppClass);
        statusBar->setObjectName("statusBar");
        statusBar->setSizeGripEnabled(true);
        statusLabel = new QLabel(statusBar);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setMinimumSize(QSize(80, 0));
        currentNodeLabel = new QLabel(statusBar);
        currentNodeLabel->setObjectName("currentNodeLabel");
        currentNodeLabel->setMinimumSize(QSize(200, 0));
        currentNodeLabel->setAlignment(Qt::AlignCenter);
        startTimeLabel = new QLabel(statusBar);
        startTimeLabel->setObjectName("startTimeLabel");
        startTimeLabel->setMinimumSize(QSize(150, 0));
        startTimeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        v2raycppClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuServer->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        mainToolBar->addAction(actionStart);
        mainToolBar->addAction(actionStop);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionImport);
        mainToolBar->addAction(actionAdd);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionSettings);

        retranslateUi(v2raycppClass);

        QMetaObject::connectSlotsByName(v2raycppClass);
    } // setupUi

    void retranslateUi(QMainWindow *v2raycppClass)
    {
        v2raycppClass->setWindowTitle(QCoreApplication::translate("v2raycppClass", "v2raycpp", nullptr));
        actionStart->setText(QCoreApplication::translate("v2raycppClass", "\345\220\257\345\212\250", nullptr));
#if QT_CONFIG(tooltip)
        actionStart->setToolTip(QCoreApplication::translate("v2raycppClass", "\345\220\257\345\212\250\344\273\243\347\220\206", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionStart->setShortcut(QCoreApplication::translate("v2raycppClass", "Ctrl+R", nullptr));
#endif // QT_CONFIG(shortcut)
        actionStop->setText(QCoreApplication::translate("v2raycppClass", "\345\201\234\346\255\242", nullptr));
#if QT_CONFIG(tooltip)
        actionStop->setToolTip(QCoreApplication::translate("v2raycppClass", "\345\201\234\346\255\242\344\273\243\347\220\206", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionStop->setShortcut(QCoreApplication::translate("v2raycppClass", "Ctrl+T", nullptr));
#endif // QT_CONFIG(shortcut)
        actionImport->setText(QCoreApplication::translate("v2raycppClass", "\345\257\274\345\205\245", nullptr));
#if QT_CONFIG(tooltip)
        actionImport->setToolTip(QCoreApplication::translate("v2raycppClass", "\344\273\216\345\211\252\350\264\264\346\235\277\345\257\274\345\205\245", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAdd->setText(QCoreApplication::translate("v2raycppClass", "\346\267\273\345\212\240", nullptr));
#if QT_CONFIG(tooltip)
        actionAdd->setToolTip(QCoreApplication::translate("v2raycppClass", "\346\267\273\345\212\240\346\234\215\345\212\241\345\231\250", nullptr));
#endif // QT_CONFIG(tooltip)
        actionSettings->setText(QCoreApplication::translate("v2raycppClass", "\350\256\276\347\275\256", nullptr));
#if QT_CONFIG(tooltip)
        actionSettings->setToolTip(QCoreApplication::translate("v2raycppClass", "\347\263\273\347\273\237\350\256\276\347\275\256", nullptr));
#endif // QT_CONFIG(tooltip)
        menuFile->setTitle(QCoreApplication::translate("v2raycppClass", "\346\226\207\344\273\266", nullptr));
        menuServer->setTitle(QCoreApplication::translate("v2raycppClass", "\346\234\215\345\212\241\345\231\250", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("v2raycppClass", "\345\270\256\345\212\251", nullptr));
        mainToolBar->setWindowTitle(QCoreApplication::translate("v2raycppClass", "\345\267\245\345\205\267\346\240\217", nullptr));
        statusLabel->setText(QCoreApplication::translate("v2raycppClass", "\346\234\252\350\277\236\346\216\245", nullptr));
        currentNodeLabel->setText(QCoreApplication::translate("v2raycppClass", "\345\275\223\345\211\215\350\212\202\347\202\271: \346\227\240", nullptr));
        startTimeLabel->setText(QCoreApplication::translate("v2raycppClass", "\345\220\257\345\212\250\346\227\266\351\227\264: --", nullptr));
    } // retranslateUi

};

namespace Ui {
    class v2raycppClass: public Ui_v2raycppClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_V2RAYCPP_H
