// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#pragma once

#include <FL/Fl_Widget.H>

namespace xconnect {

class ToggleSwitch : public Fl_Widget {
public:
    ToggleSwitch(int x, int y, int w, int h, const char* label = nullptr);

    void set(bool v);
    [[nodiscard]] bool value() const noexcept { return on_; }

    void draw() override;
    int  handle(int e) override;

private:
    bool on_ = false;
};

} // namespace xconnect
