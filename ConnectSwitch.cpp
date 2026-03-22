#include "ConnectSwitch.h"

#include <QPaintEvent>
#include <QPainter>

namespace {
// Keep in sync with style/main.qss SimpleCard pill colors (#2A354D idle icon bg, #1C60F9 accent)
constexpr int kSwitchWidth = 36;
constexpr int kSwitchHeight = 20;
} // namespace

ConnectSwitch::ConnectSwitch(QWidget* parent)
    : QAbstractButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedSize(kSwitchWidth, kSwitchHeight);
}

QSize ConnectSwitch::sizeHint() const
{
    return QSize(kSwitchWidth, kSwitchHeight);
}

QSize ConnectSwitch::minimumSizeHint() const
{
    return QSize(kSwitchWidth, kSwitchHeight);
}

void ConnectSwitch::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const bool on = isChecked();
    const QRect outer = rect();

    const QColor trackOff(0x2A, 0x35, 0x4D);
    const QColor trackOn(0x1C, 0x60, 0xF9);
    const QColor borderOff(0x3A, 0x4A, 0x6B);
    const QColor borderOn(0x4A, 0x7C, 0xFF);
    const QColor thumb(0xFF, 0xFF, 0xFF);

    constexpr int inset = 1;
    QRect track = outer.adjusted(inset, inset, -inset, -inset);
    const qreal radius = track.height() / 2.0;

    p.setPen(QPen(on ? borderOn : borderOff, 1));
    p.setBrush(on ? trackOn : trackOff);
    p.drawRoundedRect(track, radius, radius);

    const int thumbDiameter = outer.height() - 6;
    constexpr int marginX = 3;
    const int xLeft = track.left() + marginX;
    const int xRight = track.right() - thumbDiameter - marginX + 1;
    const int x = on ? xRight : xLeft;
    const int y = outer.center().y() - thumbDiameter / 2;

    p.setPen(Qt::NoPen);
    p.setBrush(thumb);
    p.drawEllipse(QRect(x, y, thumbDiameter, thumbDiameter));
}
