// Xconnect - Gestor de VPN para Devuan/Loc-OS (32 bits, SysVinit)
// Inspirado en FLConnect 1.0-4 (GPL-3.0)
// Copyright (C) 2026 linuxcero518
// Licencia: GPL-3.0. Ver LICENSE.

#pragma once

#include "xconnect/Profile.hpp"
#include <FL/Fl_Window.H>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

class Fl_Hold_Browser;
class Fl_Box;
class Fl_Multiline_Output;
class Fl_Text_Buffer;
class Fl_Text_Display;
class Fl_Button;

namespace xconnect {

class ToggleSwitch;

class MainWindow : public Fl_Window {
public:
    using ToggleFn = std::function<void()>;
    using ImportFn = std::function<void(VpnType)>;
    using DeleteFn = std::function<void()>;
    using SelectFn = std::function<void(std::optional<std::size_t>)>;

    MainWindow();
    ~MainWindow() override;

    void setProfiles(const std::vector<Profile>& profiles);
    void setSelected(std::optional<std::size_t> idx);
    void setToggleState(bool on);
    void setStatus(std::string_view text, bool connected);
    void setInfo(std::string_view text);
    void appendLog(std::string_view text);
    void setToggleEnabled(bool enabled);

    void onToggle(ToggleFn fn) { toggleFn_ = std::move(fn); }
    void onImport(ImportFn fn) { importFn_ = std::move(fn); }
    void onDelete(DeleteFn fn) { deleteFn_ = std::move(fn); }
    void onSelect(SelectFn fn) { selectFn_ = std::move(fn); }

    [[nodiscard]] std::optional<std::size_t> selected() const;

private:
    static void cbToggle(Fl_Widget*, void*);
    static void cbImportWg(Fl_Widget*, void*);
    static void cbImportOv(Fl_Widget*, void*);
    static void cbDelete(Fl_Widget*, void*);
    static void cbList(Fl_Widget*, void*);

    ToggleSwitch*        toggle_  = nullptr;
    Fl_Hold_Browser*     list_    = nullptr;
    Fl_Box*              status_  = nullptr;
    Fl_Multiline_Output* info_    = nullptr;
    Fl_Text_Display*     logview_ = nullptr;
    Fl_Text_Buffer*      logbuf_  = nullptr;
    Fl_Button*           btnWg_   = nullptr;
    Fl_Button*           btnOv_   = nullptr;
    Fl_Button*           btnDel_  = nullptr;

    ToggleFn toggleFn_;
    ImportFn importFn_;
    DeleteFn deleteFn_;
    SelectFn selectFn_;
};

} // namespace xconnect