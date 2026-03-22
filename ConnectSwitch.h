#pragma once

#include <QAbstractButton>

/// Compact iOS-style on/off switch for server cards (design parity with Figma).
/// Visual colors are documented in style/main.qss next to SimpleCard QLabel#simpleCardTag rules.
class ConnectSwitch : public QAbstractButton
{
public:
    explicit ConnectSwitch(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};
