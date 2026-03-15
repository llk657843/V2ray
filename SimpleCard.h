#ifndef SIMPLECARD_H
#define SIMPLECARD_H

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

class SimpleCard : public QFrame
{
public:
    explicit SimpleCard(QWidget *parent = nullptr);

    void setIconText(const QString &txt);
    void setTitle(const QString &txt);
    void setSubText(const QString &txt);

private:
    QLabel *m_icon = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_sub = nullptr;
};

#endif // SIMPLECARD_H
