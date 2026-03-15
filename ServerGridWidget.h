#ifndef SERVERGRIDWIDGET_H
#define SERVERGRIDWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include <QVector>
#include "ServerCardWidget.h"

class ServerGridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ServerGridWidget(QWidget *parent = nullptr);
    ~ServerGridWidget();

    void addServer(const QString &name, int latency, const QString &protocol, const QString &flagPath, bool connected);
    void clearServers();
    int serverCount() const;
    ServerCardWidget* getServer(int index) const;

signals:
    void serverClicked(int index);
    void serverToggled(int index, bool connected);

private slots:
    void onCardClicked();
    void onCardToggled(bool checked);

private:
    void setupUi();
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    QScrollArea *m_scrollArea;
    QWidget *m_containerWidget;
    QGridLayout *m_gridLayout;
    QVector<ServerCardWidget*> m_cards;
    int m_columns;
};

#endif
