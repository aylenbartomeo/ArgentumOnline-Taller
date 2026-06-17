#!/usr/bin/env bash
# =============================================================================
#  uninstaller.sh — Desinstalador para Argentum Online
#  Invocado por: make uninstall (desde la raíz del proyecto)
#
#  make uninstall pasa --purge automáticamente, eliminando todo.
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Configuración (debe ser idéntica a installer.sh)
# ---------------------------------------------------------------------------
APP_NAME="argentum_online"
APP_DISPLAY_NAME="Argentum Online"
PURGE=false

BIN_DIR="$HOME/.local/bin"
SHARE_DIR="$HOME/.local/share/${APP_NAME}"
CONFIG_DIR="$HOME/.config/${APP_NAME}"
ICONS_DIR="$HOME/.local/share/icons/${APP_NAME}"
DESKTOP_DIR="$HOME/.local/share/applications"

APP_BINARIES=(
    argentum_online_server
    argentum_online_client
    argentum_online_editor
)

DESKTOP_FILES=(
    "${DESKTOP_DIR}/argentum_online_client.desktop"
    "${DESKTOP_DIR}/argentum_online_editor.desktop"
)

# ---------------------------------------------------------------------------
# Colores
# ---------------------------------------------------------------------------
RED='\033[0;31m'; GREEN='\033[0;32m'
YELLOW='\033[1;33m'; CYAN='\033[0;36m'; NC='\033[0m'

log_info()  { echo -e "${CYAN}[INFO]${NC}  $*"; }
log_ok()    { echo -e "${GREEN}[OK]${NC}    $*"; }
log_warn()  { echo -e "${YELLOW}[WARN]${NC}  $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*" >&2; }
die()       { log_error "$*"; exit 1; }

# ---------------------------------------------------------------------------
# Parseo de argumentos
# ---------------------------------------------------------------------------
while [[ $# -gt 0 ]]; do
    case "$1" in
        --purge) PURGE=true; shift ;;
        *)       die "Argumento desconocido: $1" ;;
    esac
done

# ---------------------------------------------------------------------------
# Confirmación interactiva
# ---------------------------------------------------------------------------
echo ""
echo -e "${RED}╔══════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${RED}║            Desinstalador de ${APP_DISPLAY_NAME}                   ${NC}"
echo -e "${RED}╚══════════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo "  Se eliminarán:"
for bin_name in "${APP_BINARIES[@]}"; do
    echo "    - ${BIN_DIR}/${bin_name}"
done
for desktop_file in "${DESKTOP_FILES[@]}"; do
    echo "    - ${desktop_file}  (ícono de escritorio)"
done
echo "    - ${ICONS_DIR}/  (íconos PNG)"
echo "    - ${SHARE_DIR}/  (assets: maps, resources, game_data)"

if $PURGE; then
    echo "    - ${CONFIG_DIR}/  (configuración)"
    echo "    - auth_data/, users_data/, worlds/  (bases de datos locales)"
else
    echo "    - ${CONFIG_DIR}/  (conservado; usá --purge para eliminarlo)"
fi

echo ""
read -r -p "¿Confirmás la desinstalación? [s/N]: " CONFIRM
[[ "$CONFIRM" == "s" || "$CONFIRM" == "S" ]] \
    || { log_warn "Desinstalación cancelada."; exit 0; }
echo ""

# ---------------------------------------------------------------------------
# PASO 1 — Eliminar binarios
# ---------------------------------------------------------------------------
log_info "--- Paso 1/5: Eliminando binarios ---"

REMOVED=0
for bin_name in "${APP_BINARIES[@]}"; do
    target="${BIN_DIR}/${bin_name}"
    if [[ -f "$target" ]]; then
        rm -f "$target"
        log_info "  Eliminado: ${target}"
        REMOVED=$((REMOVED + 1))
    else
        log_warn "  No encontrado: ${target}"
    fi
done

[[ $REMOVED -gt 0 ]] \
    && log_ok "${REMOVED} binario(s) eliminado(s)." \
    || log_warn "No se eliminó ningún binario (¿ya desinstalado?)."

# ---------------------------------------------------------------------------
# PASO 2 — Eliminar entradas .desktop (íconos de escritorio)
# ---------------------------------------------------------------------------
log_info "--- Paso 2/5: Eliminando íconos de escritorio ---"

for desktop_file in "${DESKTOP_FILES[@]}"; do
    if [[ -f "$desktop_file" ]]; then
        rm -f "$desktop_file"
        log_info "  Eliminado: ${desktop_file}"
    else
        log_warn "  No encontrado: ${desktop_file}"
    fi
done

command -v update-desktop-database &>/dev/null \
    && update-desktop-database "${DESKTOP_DIR}" 2>/dev/null || true

log_ok "Íconos de escritorio eliminados."

# ---------------------------------------------------------------------------
# PASO 3 — Eliminar íconos PNG
# ---------------------------------------------------------------------------
log_info "--- Paso 3/5: Eliminando íconos PNG ---"

if [[ -d "${ICONS_DIR}" ]]; then
    rm -rf "${ICONS_DIR}"
    log_ok "Eliminado: ${ICONS_DIR}"
else
    log_warn "No encontrado: ${ICONS_DIR}"
fi

# ---------------------------------------------------------------------------
# PASO 4 — Eliminar assets (share)
# ---------------------------------------------------------------------------
log_info "--- Paso 4/5: Eliminando assets ---"

if [[ -d "${SHARE_DIR}" ]]; then
    rm -rf "${SHARE_DIR}"
    log_ok "Eliminado: ${SHARE_DIR}"
else
    log_warn "No encontrado: ${SHARE_DIR}"
fi

# ---------------------------------------------------------------------------
# PASO 5 — Configuración y bases de datos (sólo con --purge)
# ---------------------------------------------------------------------------
log_info "--- Paso 5/5: Configuración y bases de datos ---"

if $PURGE; then
    if [[ -d "${CONFIG_DIR}" ]]; then
        rm -rf "${CONFIG_DIR}"
        log_ok "Eliminado: ${CONFIG_DIR}"
    else
        log_warn "No encontrado: ${CONFIG_DIR}"
    fi

    for db_dir in auth_data users_data worlds; do
        if [[ -d "$db_dir" ]]; then
            rm -rf "$db_dir"
            log_info "  BD eliminada: ${db_dir}/"
        fi
    done
    log_ok "Configuración y bases de datos eliminadas."
else
    log_info "Configuración conservada en ${CONFIG_DIR}. Usá --purge para eliminarla."
fi

# ---------------------------------------------------------------------------
# Revertir entrada de PATH en el shell RC
# ---------------------------------------------------------------------------
SHELL_RC=""
[[ -f "$HOME/.bashrc" ]] && SHELL_RC="$HOME/.bashrc"
[[ -f "$HOME/.zshrc"  ]] && SHELL_RC="$HOME/.zshrc"

if [[ -n "$SHELL_RC" ]] \
   && grep -q "Agregado por el instalador de ${APP_NAME}" "$SHELL_RC" 2>/dev/null; then
    sed -i "/# Agregado por el instalador de ${APP_NAME}/{N;d;}" "$SHELL_RC"
    log_info "Entrada de PATH eliminada de ${SHELL_RC}."
fi

# ---------------------------------------------------------------------------
# Limpiar directorio build/ (pregunta)
# ---------------------------------------------------------------------------
if [[ -d "build" ]]; then
    echo ""
    read -r -p "¿Eliminás también el directorio build/? [s/N]: " CLEAN_BUILD
    if [[ "$CLEAN_BUILD" == "s" || "$CLEAN_BUILD" == "S" ]]; then
        rm -rf build/
        log_ok "build/ eliminado."
    fi
fi

# ---------------------------------------------------------------------------
# Resumen final
# ---------------------------------------------------------------------------
echo ""
echo -e "${GREEN}╔══════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║        ${APP_DISPLAY_NAME} desinstalado correctamente ✓           ${NC}"
echo -e "${GREEN}╚══════════════════════════════════════════════════════════════════╝${NC}"
echo ""
