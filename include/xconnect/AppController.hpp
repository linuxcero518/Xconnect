// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#pragma once

#include "xconnect/OpenVpnBackend.hpp"
#include "xconnect/ProfileManager.hpp"
#include "xconnect/WireGuardBackend.hpp"
#include <optional>

namespace xconnect {

class MainWindow;

class AppController {
public:
    AppController(MainWindow& view, ProfileManager& model);

    void bind();
    void refresh();

private:
    void scheduleRefresh();
    static void timerCb(void* self);

    void onToggle();
    void onImport(VpnType type);
    void onDelete();
    void onSelect(std::optional<std::size_t> idx);

    [[nodiscard]] VpnBackend& backendFor(VpnType t) noexcept;
    void updateDetail();
    void refreshActiveStates();

    MainWindow&      view_;
    ProfileManager&  model_;
    WireGuardBackend wg_;
    OpenVpnBackend   ov_;
    std::optional<std::size_t> selected_;
};

} // namespace xconnect
