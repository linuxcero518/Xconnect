// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include "xconnect/OpenVpnBackend.hpp"
#include "xconnect/PrivilegedRunner.hpp"

#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

namespace xconnect {

namespace {

namespace fs = std::filesystem;

fs::path pidFile(const Profile& p)  { return fs::path(p.file).replace_extension(".pid"); }
fs::path logFile(const Profile& p)  { return fs::path(p.file).replace_extension(".log"); }
fs::path credFile(const Profile& p) { return fs::path(p.file).replace_extension(".cred"); }

int readPid(const fs::path& pidf) {
    std::ifstream f(pidf);
    int pid = 0;
    if (f && (f >> pid) && pid > 0) return pid;
    return 0;
}

bool pidAlive(int pid) {
    return pid > 0 && ::kill(pid, 0) == 0;
}

bool pidFileAlive(const fs::path& pidf) {
    return pidAlive(readPid(pidf));
}

bool ovpnSucceeded(const fs::path& logf, std::string& err) {
    std::ifstream f(logf);
    if (!f) { err = "no se pudo abrir el log de OpenVPN"; return false; }

    std::vector<std::string> lines;
    lines.reserve(96);
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(std::move(line));
        if (lines.size() > 96) lines.erase(lines.begin());
    }

    static constexpr const char* fatals[] = {
        "AUTH_FAILED",
        "Exiting due to fatal error",
        "Cannot resolve host address",
        "Options error",
        "Cannot open TUN/TAP",
        "TLS Error",
    };

    std::string tail;
    tail.reserve(lines.size() * 64);
    for (const auto& l : lines) { tail += l; tail += '\n'; }

    if (tail.find("Initialization Sequence Completed") != std::string::npos)
        return true;

    for (const char* pat : fatals) {
        if (auto pos = tail.find(pat); pos != std::string::npos) {
            auto a = tail.rfind('\n', pos);
            a = (a == std::string::npos) ? 0 : a + 1;
            auto b = tail.find('\n', pos);
            b = (b == std::string::npos) ? tail.size() : b;
            err = tail.substr(a, b - a);
            return false;
        }
    }
    err.clear();
    return false;
}

} // namespace

bool OpenVpnBackend::isActive(const Profile& p) const {
    return pidFileAlive(pidFile(p));
}

bool OpenVpnBackend::activate(Profile& p, std::string& error) {
    const auto pidf = pidFile(p);
    const auto logf = logFile(p);

    if (pidFileAlive(pidf)) {
        p.active = true;
        return true;
    }

    { std::ofstream(logf, std::ios::trunc); }

    const std::string rawCmd =
        "openvpn --daemon --config " + shellQuote(p.file) +
        " --writepid " + shellQuote(pidf.string()) +
        " --log-append " + shellQuote(logf.string());
    bool used = false;
    int rc = 0;
    (void)runPrivileged(rawCmd, rc, used);
    if (rc != 0) {
        error = "openvpn no arranco (codigo " + std::to_string(rc) + ")";
        return false;
    }

    for (int i = 0; i < 30; ++i) {
        std::string perr;
        if (ovpnSucceeded(logf, perr)) { p.active = true; return true; }
        if (!perr.empty()) {
            error = perr;
            if (int pid = readPid(pidf); pidAlive(pid)) ::kill(pid, SIGTERM);
            fs::remove(pidf);
            p.active = false;
            return false;
        }
        if (i > 4 && !pidFileAlive(pidf)) {
            error = "openvpn termino inesperadamente (revisa el log)";
            p.active = false;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    error = "tiempo de espera agotado (revisa el log)";
    if (int pid = readPid(pidf); pidAlive(pid)) ::kill(pid, SIGTERM);
    fs::remove(pidf);
    p.active = false;
    return false;
}

bool OpenVpnBackend::deactivate(Profile& p, std::string& error) {
    const auto pidf = pidFile(p);
    const int pid = readPid(pidf);
    if (pidAlive(pid)) {
        bool used = false;
        int rc = 0;
        std::string out = runPrivileged("kill -TERM " + std::to_string(pid), rc, used);
        if (rc != 0) {
            error = "no se pudo enviar SIGTERM al proceso openvpn: " + out;
            return false;
        }
    }
    fs::remove(pidf);
    fs::remove(credFile(p));
    p.active = false;
    return true;
}

} // namespace xconnect