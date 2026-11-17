// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include "xconnect/WireGuardBackend.hpp"
#include "xconnect/PrivilegedRunner.hpp"

#include <filesystem>

namespace xconnect {

namespace {

std::string ifaceOf(const Profile& p) {
    return std::filesystem::path(p.file).stem().string();
}

} // namespace

bool WireGuardBackend::isActive(const Profile& p) const {
    int rc = 0;
    bool used = false;
    (void)runPrivileged("wg show " + shellQuote(ifaceOf(p)) + " dump", rc, used);
    return rc == 0;
}

bool WireGuardBackend::activate(Profile& p, std::string& error) {
    bool used = false;
    int rc = 0;

    (void)runPrivileged("modprobe wireguard", rc, used);

    std::string out = runPrivileged("wg-quick up " + shellQuote(p.file), rc, used);
    if (rc != 0) {
        error = out.empty() ? "wg-quick up fallo sin salida" : out;
        return false;
    }
    p.active = isActive(p);
    if (!p.active) {
        error = "wg-quick up termino pero la interfaz no esta activa";
        return false;
    }
    return true;
}

bool WireGuardBackend::deactivate(Profile& p, std::string& error) {
    bool used = false;
    int rc = 0;
    std::string out = runPrivileged("wg-quick down " + shellQuote(p.file), rc, used);
    p.active = isActive(p);
    if (rc != 0 && p.active) {
        error = out.empty() ? "wg-quick down fallo sin salida" : out;
        return false;
    }
    return true;
}

} // namespace xconnect