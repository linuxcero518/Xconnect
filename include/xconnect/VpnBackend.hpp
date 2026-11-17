// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#pragma once

#include "xconnect/Profile.hpp"
#include <string>
#include <string_view>

namespace xconnect {

class VpnBackend {
public:
    virtual ~VpnBackend() = default;

    [[nodiscard]] virtual bool isActive(const Profile& p) const = 0;
    [[nodiscard]] virtual bool activate(Profile& p, std::string& error) = 0;
    [[nodiscard]] virtual bool deactivate(Profile& p, std::string& error) = 0;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
};

[[nodiscard]] std::string runCapture(std::string_view cmd, int& exitCode);
[[nodiscard]] std::string shellQuote(std::string_view s);

} // namespace xconnect