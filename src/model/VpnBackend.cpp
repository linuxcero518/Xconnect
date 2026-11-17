// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include "xconnect/VpnBackend.hpp"

#include <array>
#include <cstdio>
#include <memory>
#include <sys/wait.h>

namespace xconnect {

std::string shellQuote(std::string_view s) {
    std::string r;
    r.reserve(s.size() + 2);
    r += '\'';
    for (char c : s) {
        if (c == '\'') r += "'\\''";
        else           r += c;
    }
    r += '\'';
    return r;
}

std::string runCapture(std::string_view cmd, int& exitCode) {
    std::string out;
    std::string cmdStr(cmd);
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(
        popen(cmdStr.c_str(), "r"), pclose);
    if (!pipe) {
        exitCode = -1;
        return out;
    }
    std::array<char, 512> buf{};
    while (std::fgets(buf.data(), buf.size(), pipe.get()))
        out += buf.data();

    const int rc = pclose(pipe.release());
    exitCode = WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
    return out;
}

} // namespace xconnect