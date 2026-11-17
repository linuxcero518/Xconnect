// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#pragma once

#include "xconnect/VpnBackend.hpp"

namespace xconnect {

class OpenVpnBackend final : public VpnBackend {
public:
    [[nodiscard]] bool isActive(const Profile& p) const override;
    [[nodiscard]] bool activate(Profile& p, std::string& error) override;
    [[nodiscard]] bool deactivate(Profile& p, std::string& error) override;
    [[nodiscard]] std::string_view name() const noexcept override { return "OpenVPN"; }
};

} // namespace xconnect
