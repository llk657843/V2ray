#include "SimpleCard.h"
#include <QAbstractButton>
#include <QPixmap>
#include <QDebug>
#include <QStyle>

namespace {
void polishWidget(QWidget *w)
{
    if (!w || !w->style())
        return;
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}
} // namespace

SimpleCard::SimpleCard(QWidget* parent)
    : QFrame(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setProperty("class", "card");
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(0);

    // Top area: icon | (title + status) | spacer | (lat + label)
    m_topLayout = new QHBoxLayout();
    m_topLayout->setSpacing(8);

    m_icon = new QLabel(this);
    m_icon->setObjectName("simpleCardIcon");
    m_icon->setProperty("class", "cardIcon");
    m_icon->setProperty("empty", true);
    m_icon->setFixedSize(36, 36);
    m_icon->setAlignment(Qt::AlignCenter);

    // title + status (vertical)
    QVBoxLayout* vTitleLayout = new QVBoxLayout();
    vTitleLayout->setSpacing(2);
    m_title = new QLabel(this);
    m_title->setObjectName("simpleCardTitle");
    m_title->setProperty("class", "cardTitle");
    m_status = new QLabel(this);
    m_status->setObjectName("simpleCardStatus");
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
    m_lat->setObjectName("simpleCardLat");
    m_lat->setProperty("class", "latGood");
    m_latLabel = new QLabel(this);
    m_latLabel->setObjectName("simpleCardLatCaption");
    m_latLabel->setProperty("class", "latLabel");
    m_latLabel->setText("LATENCY");

    vLatLayout->addWidget(m_lat, 0, Qt::AlignRight);
    vLatLayout->addWidget(m_latLabel, 0, Qt::AlignRight);

    m_topLayout->addWidget(m_icon);
    m_topLayout->addLayout(vTitleLayout);
    m_topLayout->addWidget(spacer);
    m_topLayout->addLayout(vLatLayout);

    m_mainLayout->addLayout(m_topLayout);

    m_line = new QFrame(this);
    m_line->setObjectName(QStringLiteral("simpleCardDivider"));
    m_line->setProperty("class", "cardLine");
    m_line->setFrameShape(QFrame::NoFrame);
    m_line->setFixedHeight(1);
    m_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_mainLayout->addWidget(m_line);

    // bottom row: tags + toggle
    QHBoxLayout* hBottom = new QHBoxLayout();
    hBottom->setSpacing(8);

    m_tagCountry = new QLabel(this);
    m_tagCountry->setObjectName("simpleCardTag1");
    m_tagCountry->setProperty("class", "tagCountry");
    m_tagCountry->setText(QString());
    m_tagCountry->hide();

    m_tagIp = new QLabel(this);
    m_tagIp->setObjectName("simpleCardTag2");
    m_tagIp->setProperty("class", "tagIp");
    m_tagIp->setText(QString());
    m_tagIp->hide();

    // push toggle to right
    QWidget* spacer2 = new QWidget(this);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_connectSwitch = new ConnectSwitch(this);
    m_connectSwitch->setObjectName("simpleCardConnectSwitch");
    m_connectSwitch->setProperty("class", "cardSwitch");

    hBottom->addWidget(m_tagCountry);
    hBottom->addWidget(m_tagIp);
    hBottom->addWidget(spacer2);
    hBottom->addWidget(m_connectSwitch);

    m_mainLayout->addLayout(hBottom);

    connect(m_connectSwitch, &QAbstractButton::toggled, this, &SimpleCard::onConnectToggled);

    for (QObject* o : children()) {
        auto* w = qobject_cast<QWidget*>(o);
        if (!w || w == m_connectSwitch)
            continue;
        w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }
}

void SimpleCard::setSelected(bool selected)
{
    setProperty("selected", selected);
    polishWidget(this);
}

bool SimpleCard::isSelected() const
{
    return property("selected").toBool();
}

void SimpleCard::setLatencyValue(int latencyMs)
{
    if (latencyMs < -1) {
        m_lat->setText(QStringLiteral("…"));
        m_lat->setProperty("class", "latNull");
    } else if (latencyMs >= 0) {
        m_lat->setText(QString::number(latencyMs) + QStringLiteral("ms"));
        if (latencyMs < 100)
            m_lat->setProperty("class", "latGood");
        else if (latencyMs < 300)
            m_lat->setProperty("class", "latMid");
        else
            m_lat->setProperty("class", "latNull");
    } else {
        m_lat->setText(QStringLiteral("---"));
        m_lat->setProperty("class", "latNull");
    }
    polishWidget(m_lat);
}

void SimpleCard::setLocationInfo(const QString& countryLine, const QString& ipLine)
{
    const QString c = countryLine.trimmed();
    const QString ip = ipLine.trimmed();
    if (c.isEmpty()) {
        m_tagCountry->clear();
        m_tagCountry->hide();
    } else {
        m_tagCountry->setText(c);
        m_tagCountry->show();
    }
    if (ip.isEmpty()) {
        m_tagIp->clear();
        m_tagIp->hide();
    } else {
        m_tagIp->setText(ip);
        m_tagIp->show();
    }
}

void SimpleCard::setNodeInfo(const QString& name, int latency, const QString& protocol, bool connected)
{
    m_title->setText(name);
    // Status 显示 protocol 或在线状态
    m_status->setText(protocol);
    setLatencyValue(latency);

    m_connectSwitch->blockSignals(true);
    m_connectSwitch->setChecked(connected);
    m_connectSwitch->blockSignals(false);
}

void SimpleCard::setFlag(const QString& flagPath)
{
    if (flagPath.isEmpty()) {
        m_icon->clear();
        m_icon->setPixmap(QPixmap());
        m_icon->setProperty("empty", true);
        polishWidget(m_icon);
        return;
    }
    if (flagPath.size() <= 4) {
        m_icon->setText(flagPath);
        m_icon->setProperty("empty", false);
        polishWidget(m_icon);
        return;
    }

    QString path = flagPath;
  
        // 回退到 qrc 路径（支持传入短名例如 "flag.png" 或 "flag"）
        path = QString(":/images/figma/%1").arg(flagPath);
    

    QPixmap pm(path);
    if (!pm.isNull()) {
        m_icon->setProperty("empty", false);
        m_icon->setPixmap(pm.scaled(m_icon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        polishWidget(m_icon);
    } else {
        m_icon->clear();
        m_icon->setPixmap(QPixmap());
        m_icon->setProperty("empty", true);
        polishWidget(m_icon);
    }
}

void SimpleCard::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit clicked();
}

void SimpleCard::onConnectToggled(bool checked)
{
    emit toggled(checked);
}