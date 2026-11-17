// Xconnect Helper - Proceso privilegiado para ejecutar comandos de VPN
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include <gio/gio.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <sys/wait.h>

namespace {

constexpr const char* kBusName = "org.xconnect.Helper";
constexpr const char* kObjectPath = "/org/xconnect/Helper";
constexpr const char* kInterface = "org.xconnect.Helper";

constexpr const char* kIntrospectionXml =
    "<node>"
    "  <interface name='org.xconnect.Helper'>"
    "    <method name='RunCommand'>"
    "      <arg type='s' name='command' direction='in'/>"
    "      <arg type='s' name='output' direction='out'/>"
    "      <arg type='i' name='exit_code' direction='out'/>"
    "    </method>"
    "  </interface>"
    "</node>";

GDBusNodeInfo* g_introspection_data = nullptr;

std::string run_command(const std::string& cmd, int& exit_code) {
    std::string out;
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        exit_code = -1;
        return "popen fallo";
    }
    std::array<char, 512> buf{};
    while (std::fgets(buf.data(), buf.size(), pipe.get())) {
        out += buf.data();
    }
    int rc = pclose(pipe.release());
    exit_code = WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
    return out;
}

void on_method_call(GDBusConnection*,
                    const gchar*,
                    const gchar*,
                    const gchar*,
                    const gchar* method_name,
                    GVariant* parameters,
                    GDBusMethodInvocation* invocation,
                    gpointer) {
    if (g_strcmp0(method_name, "RunCommand") != 0) {
        g_dbus_method_invocation_return_error(invocation,
            G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
            "Metodo desconocido: %s", method_name);
        return;
    }

    const gchar* command = nullptr;
    g_variant_get(parameters, "(&s)", &command);
    if (!command || !*command) {
        g_dbus_method_invocation_return_error(invocation,
            G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
            "Comando vacio");
        return;
    }

    int exit_code = 0;
    std::string output = run_command(command, exit_code);

    g_dbus_method_invocation_return_value(invocation,
        g_variant_new("(si)", output.c_str(), exit_code));
}

const GDBusInterfaceVTable kInterfaceVTable = {
    on_method_call,
    nullptr,
    nullptr,
    { nullptr }
};

void on_bus_acquired(GDBusConnection* connection,
                     const gchar*,
                     gpointer) {
    GError* error = nullptr;
    guint registration_id = g_dbus_connection_register_object(
        connection,
        kObjectPath,
        g_introspection_data->interfaces[0],
        &kInterfaceVTable,
        nullptr,
        nullptr,
        &error);

    if (registration_id == 0) {
        g_printerr("Error registrando objeto D-Bus: %s\n",
                   error ? error->message : "desconocido");
        g_clear_error(&error);
    }
}

void on_name_acquired(GDBusConnection*, const gchar*, gpointer) {}
void on_name_lost(GDBusConnection*, const gchar*, gpointer) {
    g_printerr("No se pudo adquirir el nombre %s en el bus del sistema.\n", kBusName);
}

} // namespace

int main(int, char**) {
    GError* error = nullptr;
    g_introspection_data = g_dbus_node_info_new_for_xml(kIntrospectionXml, &error);
    if (!g_introspection_data) {
        g_printerr("Error parseando XML de introspeccion: %s\n",
                   error ? error->message : "desconocido");
        g_clear_error(&error);
        return 1;
    }

    guint owner_id = g_bus_own_name(
        G_BUS_TYPE_SYSTEM,
        kBusName,
        G_BUS_NAME_OWNER_FLAGS_NONE,
        on_bus_acquired,
        on_name_acquired,
        on_name_lost,
        nullptr,
        nullptr);

    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    g_bus_unown_name(owner_id);
    g_main_loop_unref(loop);
    g_dbus_node_info_unref(g_introspection_data);

    return 0;
}