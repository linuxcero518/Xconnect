// Xconnect - Ejecutor privilegiado via D-Bus
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include "xconnect/PrivilegedRunner.hpp"

#include <gio/gio.h>
#include <string>

namespace xconnect {

namespace {

constexpr const char* kBusName = "org.xconnect.Helper";
constexpr const char* kObjectPath = "/org/xconnect/Helper";
constexpr const char* kInterface = "org.xconnect.Helper";

} // namespace

std::string runPrivileged(std::string_view rawCmd,
                          int& exitCode,
                          bool& usedHelper) {
    usedHelper = false;
    exitCode = -1;

    GError* error = nullptr;
    GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM,
        G_DBUS_PROXY_FLAGS_NONE,
        nullptr,
        kBusName,
        kObjectPath,
        kInterface,
        nullptr,
        &error);

    if (!proxy) {
        std::string msg = "No se pudo conectar al helper D-Bus: ";
        msg += error ? error->message : "desconocido";
        g_clear_error(&error);
        return msg;
    }

    GVariant* result = g_dbus_proxy_call_sync(
        proxy,
        "RunCommand",
        g_variant_new("(s)", std::string(rawCmd).c_str()),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error);

    if (!result) {
        std::string msg = "Llamada D-Bus fallo: ";
        msg += error ? error->message : "desconocido";
        g_clear_error(&error);
        g_object_unref(proxy);
        return msg;
    }

    const gchar* output = nullptr;
    gint helperExit = 0;
    g_variant_get(result, "(&si)", &output, &helperExit);

    std::string out = output ? output : "";
    exitCode = helperExit;
    usedHelper = true;

    g_variant_unref(result);
    g_object_unref(proxy);

    return out;
}

} // namespace xconnect