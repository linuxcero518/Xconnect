# Xconnect

Gestor de VPN ligero para Devuan/Loc-OS en 32 bits con SysVinit.

GUI en **C++20 + FLTK 1.3** que importa perfiles **WireGuard** (`.conf`) y
**OpenVPN** (`.ovpn`) al estilo NetworkManager: los archivos referenciados
por un `.ovpn` (ca, cert, key, tls-auth, tls-crypt…) se copian junto al
perfil para que no se rompan las rutas, y si el perfil pide usuario/contraseña
se solicitan al conectar de forma segura.

## Estado

🚧 **En desarrollo activo.** Reescritura desde cero inspirada en
[FLConnect 1.0-4](https://github.com/…/flconnect) (GPL-3.0), con C++20
moderno: `std::filesystem`, RAII, clases separadas para backend de VPN,
gestor de perfiles e interfaz.

## Requisitos

- Devuan 6 (Excalibur) o compatible, 32 bits (i386)
- `libfltk1.3-dev` (1.3.11-2 en Excalibur)
- `wireguard-tools`, `openvpn`, `iproute2`, `nftables`
- `g++` con soporte C++20 (GCC 14 en Excalibur)

## Compilar

```bash
make
