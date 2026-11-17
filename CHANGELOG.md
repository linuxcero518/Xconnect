# Changelog

Todas las versiones notables de Xconnect se documentan aquí.
El formato sigue [Keep a Changelog](https://keepachangelog.com/es/1.1.0/)
y el proyecto usa [Versionado Semántico](https://semver.org/lang/es/).

## [1.0.0] — 2026-11-17

### Añadido

- GUI FLTK redimensionable (600×500, mínimo 560×420) con lista de perfiles,
  interruptor ON/OFF, panel de estado y log en pantalla.
- Importación de perfiles WireGuard (`.conf`) y OpenVPN (`.ovpn`).
- Copia automática de archivos externos referenciados por un `.ovpn`
  (ca, cert, key, tls-auth, tls-crypt, pkcs12, crl-verify).
- Detección del estado real: `wg show` para WireGuard, pidfile para OpenVPN.
- Helper privilegiado `xconnect-helper` con D-Bus + polkit.
- Autenticación única por sesión de trabajo (`auth_admin_keep`).
- Archivos de integración: `.desktop`, `.policy`, `.conf`, `.service`.
- `Makefile` con targets `all`, `clean`, `run`, `install`, `remove`.
- Soporte para Devuan 6 (Excalibur) i386 con SysVinit.

### Corregido

- Segfault en los callbacks de importación por uso incorrecto de
  `user_data` en FLTK 1.3. Se sustituyó por `w->window()` para obtener
  el `MainWindow` desde cualquier widget.
- Avisos de `Fl_Text_Buffer::remove_modify_callback` al cerrar la ventana.
- Solapamiento del subtítulo con la etiqueta "Perfiles" del browser.
- Texto de estado cortado en el panel derecho; ahora usa `FL_ALIGN_WRAP`.
- Ruta del perfil saliéndose del panel; ahora usa `Fl_Multiline_Output`
  con `wrap(1)` y scroll.

### Cambiado

- Elevación de privilegios: de `sudo -n` / `pkexec` por comando a helper
  D-Bus persistente con polkit. La autenticación se pide una sola vez.
- Modelo de datos: `Profile` con `enum class VpnType` en vez de `string`.
- Gestión de archivos: `std::filesystem` en vez de `dirent.h` + `stat`.
- E/S de archivos: `std::ifstream` / `std::ofstream` en vez de `FILE*`.

### Eliminado

- Dependencia de `setuid root` en el binario de la GUI.
- Dependencia de `sudoers` para el funcionamiento normal.
- Llamadas a `pkexec` por comando (sustituidas por D-Bus).