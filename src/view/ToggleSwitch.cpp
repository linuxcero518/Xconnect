// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include "xconnect/ToggleSwitch.hpp"

#include <FL/Fl.H>
#include <FL/fl_draw.H>

namespace xconnect {

ToggleSwitch::ToggleSwitch(int x, int y, int w, int h, const char* label)
    : Fl_Widget(x, y, w, h, label) {
    box(FL_NO_BOX);
}

void ToggleSwitch::set(bool v) {
    if (v != on_) { on_ = v; redraw(); }
}

void ToggleSwitch::draw() {
    const int X = x(), Y = y(), W = w(), H = h();
    const int r = H / 2;

    fl_color(on_ ? fl_rgb_color(52, 199, 89) : fl_rgb_color(178, 178, 178));
    fl_rectf(X + r, Y, W - 2 * r, H);
    fl_pie(X, Y, H, H, 0.0, 360.0);
    fl_pie(X + W - H, Y, H, H, 0.0, 360.0);

    const int kd = H - 10;
    const int kx = on_ ? (X + W - 5 - kd) : (X + 5);
    const int ky = Y + 5;

    fl_color(fl_rgb_color(210, 210, 210));
    fl_pie(kx + 1, ky + 2, kd, kd, 0.0, 360.0);
    fl_color(FL_WHITE);
    fl_pie(kx, ky, kd, kd, 0.0, 360.0);

    fl_font(FL_HELVETICA_BOLD, 13);
    const char* t = on_ ? "ON" : "OFF";
    int tw = 0, th = 0;
    fl_measure(t, tw, th);
    const int tx = on_ ? (X + 12) : (X + W - 12 - tw);
    fl_color(FL_WHITE);
    fl_draw(t, tx, Y + (H + th) / 2 - 2);
}

int ToggleSwitch::handle(int e) {
    if (e == FL_PUSH && Fl::event_button() == FL_LEFT_MOUSE) {
        on_ = !on_;
        redraw();
        do_callback();
        return 1;
    }
    if (e == FL_KEYBOARD && (Fl::event_key() == ' ' || Fl::event_key() == FL_Enter)) {
        on_ = !on_;
        redraw();
        do_callback();
        return 1;
    }
    return Fl_Widget::handle(e);
}

} // namespace xconnect
