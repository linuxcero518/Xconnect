// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include "xconnect/ProfileManager.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace xconnect {

namespace fs = std::filesystem;

namespace {

std::string trim(std::string_view s) {
    std::size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return std::string(s.substr(a, b - a));
}

std::string sanitize(std::string_view n) {
    std::string r;
    for (char c : n) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_')
            r += c;
        else if (c == ' ')
            r += '_';
    }
    return r.empty() ? "vpn" : r;
}

bool parseOvpnLine(std::string_view t, std::string& dir, std::string& arg) {
    if (t.empty() || t[0] == '#' || t[0] == ';') return false;
    const auto sp = t.find_first_of(" \t");
    dir = (sp == std::string_view::npos) ? std::string(t) : std::string(t.substr(0, sp));
    arg = (sp == std::string_view::npos) ? std::string{} : trim(t.substr(sp + 1));
    if (arg.size() >= 2 && arg.front() == '"' && arg.back() == '"')
        arg = arg.substr(1, arg.size() - 2);
    return true;
}

bool isBlockStart(std::string_view t, std::string& tag) {
    if (t.size() > 2 && t.front() == '<' && t.back() == '>' &&
        t.find(' ') == std::string_view::npos && t[1] != '/') {
        tag = std::string(t.substr(1, t.size() - 2));
        return true;
    }
    return false;
}

} // namespace

ProfileManager::ProfileManager(fs::path dir, LogFn logger)
    : dir_(std::move(dir)), log_(std::move(logger)) {}

std::size_t ProfileManager::load() {
    profiles_.clear();
    std::error_code ec;
    fs::create_directories(dir_, ec);
    if (ec) {
        log_("No se pudo crear " + dir_.string() + ": " + ec.message());
        return 0;
    }

    for (const auto& e : fs::directory_iterator(dir_, ec)) {
        if (!e.is_regular_file()) continue;
        const auto ext = e.path().extension().string();
        Profile p;
        p.file = e.path().string();
        p.name = e.path().stem().string();
        if (ext == ".conf")      p.type = VpnType::WireGuard;
        else if (ext == ".ovpn") p.type = VpnType::OpenVpn;
        else continue;
        profiles_.push_back(std::move(p));
    }

    std::sort(profiles_.begin(), profiles_.end(),
              [](const Profile& a, const Profile& b) { return a.name < b.name; });
    return profiles_.size();
}

std::optional<fs::path>
ProfileManager::importWireGuard(const fs::path& src) {
    std::error_code ec;
    if (!fs::is_regular_file(src, ec)) {
        log_("No existe el archivo: " + src.string());
        return std::nullopt;
    }

    const std::string base = sanitize(src.stem().string());
    fs::path dst = dir_ / (base + ".conf");
    int n = 1;
    while (fs::exists(dst, ec))
        dst = dir_ / (base + "_" + std::to_string(++n) + ".conf");

    if (!fs::copy_file(src, dst, fs::copy_options::none, ec)) {
        log_("No se pudo copiar a " + dst.string() + ": " + ec.message());
        return std::nullopt;
    }
    fs::permissions(dst, fs::perms::owner_read | fs::perms::owner_write,
                    fs::perm_options::replace, ec);

    {
        std::ifstream f(dst);
        std::string line;
        bool hasIface = false;
        while (std::getline(f, line))
            if (trim(line) == "[Interface]") { hasIface = true; break; }
        if (!hasIface)
            log_("Aviso: " + dst.filename().string() + " no contiene [Interface].");
    }

    log_("Perfil WireGuard importado: " + dst.filename().string());
    return dst;
}

std::optional<fs::path>
ProfileManager::importOpenVpn(const fs::path& src) {
    std::error_code ec;
    if (!fs::is_regular_file(src, ec)) {
        log_("No existe el archivo: " + src.string());
        return std::nullopt;
    }

    const std::string base = sanitize(src.stem().string());
    fs::path dst = dir_ / (base + ".ovpn");
    int n = 1;
    while (fs::exists(dst, ec))
        dst = dir_ / (base + "_" + std::to_string(++n) + ".ovpn");
    const std::string finalBase = dst.stem().string();

    std::ifstream in(src);
    std::ofstream out(dst, std::ios::trunc);
    if (!in || !out) {
        log_("No se pudo abrir " + src.string() + " o " + dst.string());
        return std::nullopt;
    }

    const fs::path srcdir = src.parent_path();
    std::string line, blockTag;
    bool inBlock = false, hasRemote = false;
    int copied = 0;

    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const std::string t = trim(line);

        if (inBlock) {
            out << line << '\n';
            if (t == "</" + blockTag + ">") inBlock = false;
            continue;
        }

        std::string tag;
        if (isBlockStart(t, tag)) {
            blockTag = tag;
            inBlock = true;
            out << line << '\n';
            continue;
        }

        std::string dir, arg;
        if (parseOvpnLine(t, dir, arg)) {
            if (dir == "remote") hasRemote = true;
            if (isOvpnFileDirective(dir) && !arg.empty() && arg != "[inline]") {
                fs::path fpath = arg;
                if (fpath.is_relative()) fpath = srcdir / fpath;
                if (auto copiedPath = copyExternalFile(fpath, finalBase, dir)) {
                    ++copied;
                    out << dir << ' ' << copiedPath->string() << '\n';
                    continue;
                }
                log_("Aviso: no se copio " + dir + " -> " + arg +
                     " (se conserva la ruta original)");
            }
        }
        out << line << '\n';
    }
    in.close(); out.close();

    fs::permissions(dst, fs::perms::owner_read | fs::perms::owner_write,
                    fs::perm_options::replace, ec);

    if (copied > 0)
        log_("Archivos externos integrados al perfil: " + std::to_string(copied));
    if (!hasRemote)
        log_("Aviso: " + dst.filename().string() + " no contiene 'remote'.");
    log_("Perfil OpenVPN importado: " + dst.filename().string());
    return dst;
}

void ProfileManager::remove(const Profile& p) {
    std::error_code ec;
    fs::remove(p.file, ec);

    const fs::path base = fs::path(p.file).replace_extension();
    for (const char* ext : {".pid", ".log", ".cred"})
        fs::remove(base.string() + ext, ec);

    const std::string pre = fs::path(p.file).stem().string() + "_";
    for (const auto& e : fs::directory_iterator(dir_, ec)) {
        if (e.is_regular_file() &&
            e.path().filename().string().rfind(pre, 0) == 0)
            fs::remove(e.path(), ec);
    }
}

bool ProfileManager::isOvpnFileDirective(std::string_view directive) {
    static constexpr std::string_view dirs[] = {
        "ca", "cert", "key", "dh",
        "tls-auth", "tls-crypt", "tls-crypt-v2",
        "pkcs12", "crl-verify", "auth-user-pass",
    };
    return std::find(std::begin(dirs), std::end(dirs), directive) != std::end(dirs);
}

std::optional<fs::path>
ProfileManager::copyExternalFile(const fs::path& src,
                                 std::string_view base,
                                 std::string_view directive) {
    std::error_code ec;
    if (!fs::is_regular_file(src, ec)) return std::nullopt;

    const fs::path dst = dir_ / (std::string(base) + "_" +
                                 std::string(directive) + "_" +
                                 src.filename().string());
    if (!fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec))
        return std::nullopt;

    fs::permissions(dst, fs::perms::owner_read | fs::perms::owner_write,
                    fs::perm_options::replace, ec);
    return dst;
}

} // namespace xconnect
