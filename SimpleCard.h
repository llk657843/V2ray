#ifndef SIMPLECARD_H
#define SIMPLECARD_H

#include <QFrame>
#include <QLabel>
#include "ConnectSwitch.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QMouseEvent>

class SimpleCard : public QFrame
{
    Q_OBJECT
public:
    explicit SimpleCard(QWidget* parent = nullptr);

    // ???????????name ???? title??latency ?????ms????protocol ???? status ?????connected ?????????
    void setNodeInfo(const QString& name, int latency, const QString& protocol, bool connected);
    /// ?????????????latency < -1 ?????????????? ??????-1 ??????
    void setLatencyValue(int latencyMs);
    /// ????/????????????? IP?????? "???? (US)" ?? "203.0.113.1"??
    void setLocationInfo(const QString& countryLine, const QString& ipLine);
    void setFlag(const QString& flagPath); // ????? emoji ????? pixmap ????
    void setSelected(bool selected);
    bool isSelected() const;

signals:
    void clicked();
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onConnectToggled(bool checked);

private:
    QLabel* m_icon = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_lat = nullptr;
    QLabel* m_latLabel = nullptr;
    QLabel* m_tagCountry = nullptr;
    QLabel* m_tagIp = nullptr;
    ConnectSwitch* m_connectSwitch = nullptr;
    QFrame* m_line = nullptr;
    QHBoxLayout* m_topLayout = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;
};

#endif // SIMPLECARD_H