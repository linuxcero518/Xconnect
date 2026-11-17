// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include "xconnect/AppController.hpp"
#include "xconnect/MainWindow.hpp"
#include "xconnect/ProfileManager.hpp"

#include <FL/Fl.H>
#include <cstdlib>
#include <filesystem>
#include <string>

int main(int argc, char** argv) {
    Fl::scheme("gtk+");

    const char* home = std::getenv("HOME");
    std::filesystem::path base = home ? home : "/root";
    base /= ".config/xconnect/profiles";

    xconnect::MainWindow view;
    xconnect::ProfileManager model(base, [&view](std::string_view s) {
        view.appendLog(s);
    });
    xconnect::AppController ctrl(view, model);

    ctrl.bind();
    ctrl.refresh();

    view.show(argc, argv);
    return Fl::run();
}
