# Xconnect

Gestor de VPN ligero para **Devuan / Loc-OS** en 32 bits con **SysVinit**.

GUI en **C++20 + FLTK 1.3** que importa perfiles **WireGuard** (`.conf`) y
**OpenVPN** (`.ovpn`) al estilo NetworkManager: los archivos referenciados
por un `.ovpn` (ca, cert, key, tls-auth, tls-crypt…) se copian junto al
perfil para que no se rompan las rutas.

La elevación de privilegios se hace con **D-Bus + polkit**: la primera
operación pide autenticación una sola vez, y las siguientes usan la caché
de polkit (`auth_admin_keep`) mientras el helper siga vivo. No se necesita
`sudo` ni configurar `sudoers`.

## Estado

✅ **v1.0.0 — primera release estable.**

Probado en Devuan 6 (Excalibur) i386 con SysVinit y FLTK 1.3.11.
Compatible con Loc-OS Linux y cualquier derivado de Devuan sin systemd.

## Características

- Interfaz FLTK redimensionable (600×500 por defecto, mínimo 560×420).
- Importación de perfiles WireGuard (`.conf`) y OpenVPN (`.ovpn`).
- Copia automática de archivos externos referenciados por un `.ovpn`.
- Conexión / desconexión con un interruptor ON/OFF.
- Detección del estado real: `wg show` para WireGuard, pidfile para OpenVPN.
- Log en pantalla con los comandos ejecutados y sus errores.
- Autenticación única vía polkit; sin `sudo`, sin `sudoers`, sin setuid.
- Sin dependencias de systemd.

## Requisitos

### Compilación

- `g++` con soporte C++20 (GCC 14 o superior)
- `libfltk1.3-dev` (1.3.11-2 en Excalibur)
- `libglib2.0-dev` (para GDBus)
- `pkg-config`

### Ejecución

- `wireguard-tools` (para `wg` y `wg-quick`)
- `openvpn` (para perfiles `.ovpn`)
- `iproute2`
- `nftables` (para túneles con `AllowedIPs = 0.0.0.0/0`)
- `policykit-1` y un agente polkit gráfico (`xfce-polkit`, `polkit-gnome`, etc.)
- `dbus` (el bus del sistema, ya presente en Devuan)

## Compilar

Ejecuta:

    make

Genera dos binarios: `xconnect` (GUI) y `xconnect-helper` (proceso privilegiado).

## Instalar

Ejecuta:

    sudo make install

Instala:

- `/usr/bin/xconnect` — GUI
- `/usr/libexec/xconnect-helper` — helper privilegiado
- `/usr/share/applications/xconnect.desktop` — entrada de menú
- `/usr/share/polkit-1/actions/org.xconnect.Helper.policy` — política polkit
- `/usr/share/dbus-1/system.d/org.xconnect.Helper.conf` — política D-Bus
- `/usr/share/dbus-1/system-services/org.xconnect.Helper.service` — activación D-Bus

Tras instalar, recarga D-Bus y polkit:

    sudo /etc/init.d/dbus reload
    sudo /etc/init.d/polkit reload

Si `polkit` no tiene script de init en tu sistema, basta con reiniciar la
sesión gráfica o ejecutar `sudo pkill polkitd` (se relanza solo).

## Desinstalar

Ejecuta:

    sudo make remove

Borra los seis archivos que instala `make install`. No toca los perfiles
del usuario en `~/.config/xconnect/`.

## Uso

1. Abre `xconnect` desde el menú de aplicaciones o desde una terminal.
2. Pulsa **"+ Importar WireGuard"** o **"+ Importar OpenVPN"** y elige el
   archivo. Se copia a `~/.config/xconnect/profiles/`.
3. Selecciona el perfil en la lista.
4. Pulsa el interruptor. La primera vez, polkit pedirá tu contraseña de
   administrador. Las siguientes operaciones no la volverán a pedir
   mientras el helper siga vivo (~5 minutos de caché de polkit).
5. Para desconectar, pulsa el interruptor de nuevo.

Cerrar la ventana no baja una conexión activa: desconecta con el interruptor
antes de cerrar.

## Estructura del proyecto

    Xconnect/
    ├── data/                  # .desktop, .policy, .conf, .service
    ├── include/xconnect/      # headers públicos
    ├── src/
    │   ├── controller/        # AppController (MVC)
    │   ├── helper/            # xconnect-helper (proceso privilegiado)
    │   ├── model/             # ProfileManager, backends, D-Bus runner
    │   ├── view/              # MainWindow, ToggleSwitch
    │   └── main.cpp
    ├── Makefile
    ├── LICENCE
    └── README.md

## Notas sobre bloqueos geográficos

WARP (Cloudflare) es un acelerador de red, no una VPN de privacidad ni una
herramienta para eludir bloqueos geográficos. Si vives en un país con
restricciones de red, WARP puede no ser suficiente para acceder a sitios
geobloqueados. En ese caso, usa un proveedor de VPN comercial que ofrezca
WireGuard u OpenVPN, o monta tu propio servidor en un VPS fuera de tu país.
Xconnect importa cualquier `.conf` u `.ovpn` estándar, así que funciona con
cualquier proveedor.

## Licencia

GPL-3.0. Ver `LICENCE`.

## Atribución

Inspirado en FLConnect 1.0-4 (GPL-3.0). Este proyecto es una reescritura
independiente en C++20, no un fork directo. Se han reutilizado ideas de
diseño (importación estilo NetworkManager, interruptor ON/OFF) pero no
código.