// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#pragma once

#include <string>

namespace xconnect {

enum class VpnType { WireGuard, OpenVpn };

struct Profile {
    std::string name;
    std::string file;
    VpnType     type = VpnType::WireGuard;
    bool        active = false;
};

} // namespace xconnect
