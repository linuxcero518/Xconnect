// Xconnect - Ejecutor privilegiado via D-Bus
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#pragma once

#include <string>
#include <string_view>

namespace xconnect {

// Ejecuta un comando a traves del helper privilegiado (D-Bus + polkit).
// La primera llamada pedira autenticacion; las siguientes usan la cache
// de polkit (auth_admin_keep) mientras el helper siga vivo.
[[nodiscard]] std::string runPrivileged(std::string_view rawCmd,
                                        int& exitCode,
                                        bool& usedHelper);

} // namespace xconnect