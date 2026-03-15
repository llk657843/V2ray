#ifndef SIMPLECARD_H
#define SIMPLECARD_H

#include <QFrame>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QMouseEvent>

class SimpleCard : public QFrame
{
    Q_OBJECT
public:
    explicit SimpleCard(QWidget* parent = nullptr);

    // 设置节点信息：name 显示为 title，latency 为延迟（ms），protocol 显示为 status 标签，connected 控制开关状态
    void setNodeInfo(const QString& name, int latency, const QString& protocol, bool connected);
    void setFlag(const QString& flagPath); // 简单支持 emoji 文本或 pixmap 路径

signals:
    void clicked();
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onToggleChanged(bool checked);

private:
    QLabel* m_icon = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_lat = nullptr;
    QLabel* m_latLabel = nullptr;
    QCheckBox* m_toggle = nullptr;
    QFrame* m_line = nullptr;
    QHBoxLayout* m_topLayout = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;
};

#endif // SIMPLECARD_H