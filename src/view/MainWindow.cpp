// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include "xconnect/MainWindow.hpp"
#include "xconnect/ToggleSwitch.hpp"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_draw.H>

namespace xconnect {

namespace {

Fl_Button* styleButton(Fl_Button* b, Fl_Color c) {
    b->box(FL_ROUNDED_BOX);
    b->color(c);
    b->labelcolor(FL_WHITE);
    b->labelfont(FL_HELVETICA_BOLD);
    b->labelsize(13);
    return b;
}

} // namespace

MainWindow::MainWindow() : Fl_Window(600, 500, "Xconnect") {
    color(fl_rgb_color(245, 245, 247));
    size_range(560, 420);
    const int m = 12;

    auto* title = new Fl_Box(m, m, 400, 34, "Xconnect");
    title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    title->labelfont(FL_HELVETICA_BOLD);
    title->labelsize(24);
    title->labelcolor(fl_rgb_color(30, 30, 40));

    const int topY = m + 44;
    const int topH = 260;
    const int panelW = 200;

    auto* top = new Fl_Group(m, topY, 600 - 2 * m, topH);
    top->box(FL_NO_BOX);

    list_ = new Fl_Hold_Browser(top->x(), top->y(),
                                top->w() - panelW - 8, top->h(), "Perfiles");
    list_->align(FL_ALIGN_TOP_LEFT);
    list_->labelfont(FL_HELVETICA_BOLD);
    list_->textfont(FL_HELVETICA);
    list_->textsize(13);
    list_->callback(cbList);

    const int px = list_->x() + list_->w() + 8;
    const int pw = top->w() - list_->w() - 8;

    auto* swlabel = new Fl_Box(px, top->y(), pw, 20, "Conexion");
    swlabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    swlabel->labelfont(FL_HELVETICA_BOLD);
    swlabel->labelsize(13);

    toggle_ = new ToggleSwitch(px, top->y() + 24, 140, 46);
    toggle_->callback(cbToggle);
    toggle_->tooltip("Activar / desactivar la VPN seleccionada");

    status_ = new Fl_Box(px, top->y() + 78, pw, 40, "Sin perfiles");
    status_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    status_->labelfont(FL_HELVETICA_BOLD);
    status_->labelsize(13);

    // Fl_Multiline_Output: corta cadenas largas y permite scroll/seleccion.
    info_ = new Fl_Multiline_Output(px, top->y() + 122, pw, 36, "");
    info_->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
    info_->textfont(FL_HELVETICA);
    info_->textsize(10);
    info_->textcolor(fl_rgb_color(100, 100, 110));
    info_->box(FL_DOWN_BOX);
    info_->wrap(1);  // 1 = wrap en FLTK 1.3 para Fl_Output

    const int by = top->y() + 164;
    btnWg_ = styleButton(new Fl_Button(px, by, pw, 30,
                                       "+  Importar WireGuard"),
                         fl_rgb_color(0, 122, 255));
    btnWg_->callback(cbImportWg);

    btnOv_ = styleButton(new Fl_Button(px, by + 36, pw, 30,
                                       "+  Importar OpenVPN"),
                         fl_rgb_color(88, 86, 214));
    btnOv_->callback(cbImportOv);

    btnDel_ = styleButton(new Fl_Button(px, by + 72, pw, 30,
                                        "Eliminar perfil"),
                          fl_rgb_color(200, 60, 60));
    btnDel_->callback(cbDelete);

    top->end();
    top->resizable(list_);

    const int logY = topY + topH + 8;
    const int logH = 500 - logY - m;

    auto* bottom = new Fl_Group(m, logY, 600 - 2 * m, logH);
    bottom->box(FL_NO_BOX);

    auto* loglabel = new Fl_Box(bottom->x(), bottom->y(), 150, 18, "Registro");
    loglabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    loglabel->labelfont(FL_HELVETICA_BOLD);
    loglabel->labelsize(12);

    logbuf_  = new Fl_Text_Buffer();
    logview_ = new Fl_Text_Display(bottom->x(), bottom->y() + 20,
                                   bottom->w(), bottom->h() - 20);
    logview_->buffer(logbuf_);
    logview_->textfont(FL_COURIER);
    logview_->textsize(10);
    logview_->box(FL_DOWN_BOX);

    bottom->end();
    bottom->resizable(logview_);

    end();
    resizable(top);
}

MainWindow::~MainWindow() {
    // No borrar logbuf_: Fl_Text_Display lo usa durante su destrucción.
}

void MainWindow::setProfiles(const std::vector<Profile>& profiles) {
    list_->clear();
    for (const auto& p : profiles) {
        const char* tag = (p.type == VpnType::WireGuard) ? "WireGuard" : "OpenVPN";
        const std::string line = (p.active ? "* " : "  ") + p.name +
                                 "   [" + tag + "]";
        list_->add(line.c_str());
    }
}

void MainWindow::setSelected(std::optional<std::size_t> idx) {
    if (idx && *idx < static_cast<std::size_t>(list_->size()))
        list_->value(static_cast<int>(*idx) + 1);
    else
        list_->value(0);
}

void MainWindow::setToggleState(bool on) { toggle_->set(on); }

void MainWindow::setToggleEnabled(bool enabled) {
    if (enabled) toggle_->activate();
    else         toggle_->deactivate();
}

void MainWindow::setStatus(std::string_view text, bool connected) {
    status_->copy_label(std::string(text).c_str());
    status_->labelcolor(connected ? fl_rgb_color(30, 140, 60)
                                  : fl_rgb_color(150, 30, 30));
    status_->redraw();
}

void MainWindow::setInfo(std::string_view text) {
    info_->value(std::string(text).c_str());
    info_->redraw();
}

void MainWindow::appendLog(std::string_view text) {
    if (!logbuf_) return;
    logbuf_->append(std::string(text).c_str());
    logbuf_->append("\n");
}

std::optional<std::size_t> MainWindow::selected() const {
    const int v = list_->value();
    if (v < 1) return std::nullopt;
    return static_cast<std::size_t>(v - 1);
}

static MainWindow* ownerOf(Fl_Widget* w) {
    return static_cast<MainWindow*>(w->window());
}

void MainWindow::cbToggle(Fl_Widget* w, void*) {
    auto* mw = ownerOf(w);
    if (mw && mw->toggleFn_) mw->toggleFn_();
}

void MainWindow::cbImportWg(Fl_Widget* w, void*) {
    auto* mw = ownerOf(w);
    if (mw && mw->importFn_) mw->importFn_(VpnType::WireGuard);
}

void MainWindow::cbImportOv(Fl_Widget* w, void*) {
    auto* mw = ownerOf(w);
    if (mw && mw->importFn_) mw->importFn_(VpnType::OpenVpn);
}

void MainWindow::cbDelete(Fl_Widget* w, void*) {
    auto* mw = ownerOf(w);
    if (mw && mw->deleteFn_) mw->deleteFn_();
}

void MainWindow::cbList(Fl_Widget* w, void*) {
    auto* mw = ownerOf(w);
    if (mw && mw->selectFn_) mw->selectFn_(mw->selected());
}

} // namespace xconnect