#include "SimpleCard.h"
#include <QPixmap>
#include <QDebug>

SimpleCard::SimpleCard(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("cardDynamic");
    setProperty("class", "card");
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(8);

    // Top area: icon | (title + status) | spacer | (lat + label)
    m_topLayout = new QHBoxLayout();
    m_topLayout->setSpacing(8);

    m_icon = new QLabel(this);
    m_icon->setObjectName("iconDynamic");
    m_icon->setProperty("class", "cardIcon");
    m_icon->setFixedSize(36, 36);
    m_icon->setAlignment(Qt::AlignCenter);

    // title + status (vertical)
    QVBoxLayout* vTitleLayout = new QVBoxLayout();
    vTitleLayout->setSpacing(2);
    m_title = new QLabel(this);
    m_title->setObjectName("titleDynamic");
    m_title->setProperty("class", "cardTitle");
    m_status = new QLabel(this);
    m_status->setObjectName("statusDynamic");
    m_status->setProperty("class", "statusGood");

    vTitleLayout->addWidget(m_title);
    vTitleLayout->addWidget(m_status);

    // spacer to push latency to right
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // latency column
    QVBoxLayout* vLatLayout = new QVBoxLayout();
    vLatLayout->setSpacing(2);
    m_lat = new QLabel(this);
    m_lat->setObjectName("latDynamic");
    m_lat->setProperty("class", "latGood");
    m_latLabel = new QLabel(this);
    m_latLabel->setObjectName("latLabelDynamic");
    m_latLabel->setProperty("class", "latLabel");
    m_latLabel->setText("LATENCY");

    vLatLayout->addWidget(m_lat, 0, Qt::AlignRight);
    vLatLayout->addWidget(m_latLabel, 0, Qt::AlignRight);

    m_topLayout->addWidget(m_icon);
    m_topLayout->addLayout(vTitleLayout);
    m_topLayout->addWidget(spacer);
    m_topLayout->addLayout(vLatLayout);

    m_mainLayout->addLayout(m_topLayout);

    // separator line
    m_line = new QFrame(this);
    m_line->setFrameShape(QFrame::HLine);
    m_line->setFrameShadow(QFrame::Sunken);
    m_line->setProperty("class", "cardLine");
    m_mainLayout->addWidget(m_line);

    // bottom row: tags + toggle
    QHBoxLayout* hBottom = new QHBoxLayout();
    hBottom->setSpacing(8);

    // example tag placeholders (empty by default)
    QLabel* tag1 = new QLabel(this);
    tag1->setObjectName("tag1Dynamic");
    tag1->setProperty("class", "tag");
    QLabel* tag2 = new QLabel(this);
    tag2->setObjectName("tag2Dynamic");
    tag2->setProperty("class", "tag");

    // push toggle to right
    QWidget* spacer2 = new QWidget(this);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_toggle = new QCheckBox(this);
    m_toggle->setObjectName("toggleDynamic");
    m_toggle->setProperty("class", "toggle");

    hBottom->addWidget(tag1);
    hBottom->addWidget(tag2);
    hBottom->addWidget(spacer2);
    hBottom->addWidget(m_toggle);

    m_mainLayout->addLayout(hBottom);

    // connect toggle signal
    connect(m_toggle, &QCheckBox::toggled, this, &SimpleCard::onToggleChanged);
}

void SimpleCard::setNodeInfo(const QString& name, int latency, const QString& protocol, bool connected)
{
    m_title->setText(name);
    // Status 显示 protocol 或在线状态
    m_status->setText(protocol);
    // latency 显示
    if (latency >= 0) {
        m_lat->setText(QString::number(latency) + "ms");
        if (latency < 100)
            m_lat->setProperty("class", "latGood");
        else if (latency < 300)
            m_lat->setProperty("class", "latMid");
        else
            m_lat->setProperty("class", "latNull");
    }
    else {
        m_lat->setText("---");
        m_lat->setProperty("class", "latNull");
    }

    m_toggle->setChecked(connected);
}

void SimpleCard::setFlag(const QString& flagPath)
{
    // 如果是短文本（emoji），直接 setText；否则尝试加载 pixmap
    if (flagPath.isEmpty()) {
        m_icon->setText(QString());
        return;
    }
    if (flagPath.size() <= 4) {
        m_icon->setText(flagPath);
    }
    else {
        QPixmap pm(flagPath);
        if (!pm.isNull()) {
            m_icon->setPixmap(pm.scaled(m_icon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            m_icon->setText(flagPath);
        }
    }
}

void SimpleCard::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit clicked();
}

void SimpleCard::onToggleChanged(bool checked)
{
    emit toggled(checked);
}