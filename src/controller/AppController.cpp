// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#include "xconnect/AppController.hpp"
#include "xconnect/MainWindow.hpp"
#include "xconnect/VpnBackend.hpp"

#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>
#include <filesystem>
#include <string>

namespace xconnect {

namespace {

constexpr double kRefreshSeconds = 3.0;

} // namespace

AppController::AppController(MainWindow& view, ProfileManager& model)
    : view_(view), model_(model) {}

void AppController::bind() {
    view_.onToggle([this] { onToggle(); });
    view_.onImport([this](VpnType t) { onImport(t); });
    view_.onDelete([this] { onDelete(); });
    view_.onSelect([this](std::optional<std::size_t> i) { onSelect(i); });
}

void AppController::refresh() {
    model_.load();
    refreshActiveStates();
    view_.setProfiles(model_.profiles());
    if (!selected_ && !model_.profiles().empty()) selected_ = 0;
    view_.setSelected(selected_);
    updateDetail();
    scheduleRefresh();
}

// Programa el siguiente refresco periódico. Solo se llama una vez:
// a partir de ahí, repeat_timeout se re-agenda a sí mismo.
void AppController::scheduleRefresh() {
    Fl::add_timeout(kRefreshSeconds, timerCb, this);
}

void AppController::timerCb(void* self) {
    auto* c = static_cast<AppController*>(self);
    c->refreshActiveStates();
    c->updateDetail();
    Fl::repeat_timeout(kRefreshSeconds, timerCb, self);
}

void AppController::refreshActiveStates() {
    // El modelo expone profiles_ como const; usamos updateActive() para
    // mantener la encapsulación. (Ver ProfileManager en la Entrega 1:
    // añadimos un método setActive(i, bool) — lo incluimos abajo.)
    const auto& profs = model_.profiles();
    for (std::size_t i = 0; i < profs.size(); ++i) {
        const bool active = backendFor(profs[i].type).isActive(profs[i]);
        model_.setActive(i, active);
    }
}

void AppController::updateDetail() {
    const auto& profs = model_.profiles();
    if (!selected_ || *selected_ >= profs.size()) {
        view_.setToggleEnabled(false);
        view_.setToggleState(false);
        view_.setStatus("Sin perfiles: importa un .conf u .ovpn", false);
        view_.setInfo("");
        return;
    }
    const auto& p = profs[*selected_];
    view_.setToggleEnabled(true);
    view_.setToggleState(p.active);
    view_.setStatus(p.active ? "Conectado" : "Desconectado", p.active);
    view_.setInfo(std::string(p.type == VpnType::WireGuard
                                ? "WireGuard" : "OpenVPN") + "\n" + p.file);
}

VpnBackend& AppController::backendFor(VpnType t) noexcept {
    return (t == VpnType::WireGuard) ? static_cast<VpnBackend&>(wg_)
                                     : static_cast<VpnBackend&>(ov_);
}

void AppController::onToggle() {
    if (!selected_ || *selected_ >= model_.profiles().size()) return;

    // Copia local mutable: el modelo guarda const y el backend necesita
    // modificar `active` durante la operación.
    Profile p = model_.profiles()[*selected_];
    auto& backend = backendFor(p.type);

    std::string error;
    if (p.active) {
        view_.appendLog("Desactivando " + p.name + " ...");
        if (backend.deactivate(p, error))
            view_.appendLog("Desconectado.");
        else
            view_.appendLog("Fallo: " + error);
    } else {
        view_.appendLog("Activando " + p.name + " (" +
                        std::string(backend.name()) + ") ...");
        if (backend.activate(p, error))
            view_.appendLog("Conectado.");
        else
            view_.appendLog("Fallo: " + error);
    }

    // Sincronizar el estado real con el modelo y la vista.
    model_.setActive(*selected_, p.active);
    view_.setProfiles(model_.profiles());
    view_.setSelected(selected_);
    updateDetail();
}

void AppController::onImport(VpnType type) {
    Fl_Native_File_Chooser fc;
    fc.title("Importar archivo de configuracion VPN");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.filter(type == VpnType::WireGuard
                  ? "WireGuard\t*.conf\n"
                  : "OpenVPN\t*.ovpn\n");
    if (fc.show() != 0) return;

    const std::filesystem::path src = fc.filename();
    auto dst = (type == VpnType::WireGuard)
                   ? model_.importWireGuard(src)
                   : model_.importOpenVpn(src);
    if (!dst) {
        fl_alert("No se pudo importar el archivo.");
        return;
    }

    // Recargar y seleccionar el recién importado.
    model_.load();
    refreshActiveStates();
    view_.setProfiles(model_.profiles());
    for (std::size_t i = 0; i < model_.profiles().size(); ++i) {
        if (model_.profiles()[i].file == dst->string()) {
            selected_ = i;
            break;
        }
    }
    view_.setSelected(selected_);
    updateDetail();
}

void AppController::onDelete() {
    if (!selected_ || *selected_ >= model_.profiles().size()) return;

    Profile p = model_.profiles()[*selected_];
    if (p.active) {
        if (!fl_choice("El perfil '%s' esta activo. Desconectarlo y eliminarlo?",
                       "Cancelar", "Eliminar", nullptr, p.name.c_str()))
            return;
        std::string error;
        backendFor(p.type).deactivate(p, error);
    } else {
        if (!fl_choice("Eliminar el perfil '%s'?",
                       "Cancelar", "Eliminar", nullptr, p.name.c_str()))
            return;
    }

    model_.remove(p);
    view_.appendLog("Perfil eliminado: " + p.name);
    selected_.reset();
    model_.load();
    refreshActiveStates();
    view_.setProfiles(model_.profiles());
    view_.setSelected(selected_);
    updateDetail();
}

void AppController::onSelect(std::optional<std::size_t> idx) {
    selected_ = idx;
    updateDetail();
}

} // namespace xconnect
