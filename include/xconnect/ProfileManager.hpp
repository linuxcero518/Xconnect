// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#pragma once

#include "xconnect/Profile.hpp"
#include <filesystem>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

namespace xconnect {

class ProfileManager {
public:
    using LogFn = std::function<void(std::string_view)>;

    explicit ProfileManager(std::filesystem::path dir, LogFn logger);

    std::size_t load();

    std::optional<std::filesystem::path>
    importWireGuard(const std::filesystem::path& src);

    std::optional<std::filesystem::path>
    importOpenVpn(const std::filesystem::path& src);

    void remove(const Profile& p);

    void setActive(std::size_t idx, bool active) noexcept {
        if (idx < profiles_.size()) profiles_[idx].active = active;
    }

    [[nodiscard]] const std::vector<Profile>& profiles() const noexcept { return profiles_; }
    [[nodiscard]] const std::filesystem::path& dir() const noexcept { return dir_; }

private:
    static bool isOvpnFileDirective(std::string_view directive);

    std::optional<std::filesystem::path>
    copyExternalFile(const std::filesystem::path& src,
                     std::string_view base,
                     std::string_view directive);

    std::filesystem::path dir_;
    LogFn                 log_;
    std::vector<Profile>  profiles_;
};

} // namespace xconnect
