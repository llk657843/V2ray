#include "SimpleCard.h"

SimpleCard::SimpleCard(QWidget *parent)
    : QFrame(parent)
{
    // 使用与 designer 中相近的结构：垂直布局，中间 icon 与文字居中
    setObjectName("cardDynamic");
    setProperty("class", "card");
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(12, 12, 12, 12);
    vbox->setSpacing(8);

    m_icon = new QLabel(this);
    m_icon->setAlignment(Qt::AlignCenter);
    m_icon->setObjectName("cardIconDynamic");
    m_icon->setProperty("class", "cardIcon");

    m_title = new QLabel(this);
    m_title->setAlignment(Qt::AlignCenter);
    m_title->setObjectName("cardTitleDynamic");
    m_title->setProperty("class", "cardTitle");

    m_sub = new QLabel(this);
    m_sub->setAlignment(Qt::AlignCenter);
    m_sub->setObjectName("cardSubDynamic");
    m_sub->setProperty("class", "latLabel");

    vbox->addWidget(m_icon);
    vbox->addWidget(m_title);
    vbox->addWidget(m_sub);
    vbox->addStretch();
}

void SimpleCard::setIconText(const QString &txt)
{
    m_icon->setText(txt);
}

void SimpleCard::setTitle(const QString &txt)
{
    m_title->setText(txt);
}

void SimpleCard::setSubText(const QString &txt)
{
    m_sub->setText(txt);
}
