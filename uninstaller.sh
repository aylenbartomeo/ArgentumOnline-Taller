#!/usr/bin/env bash
# =============================================================================
#  uninstaller.sh — Desinstalador para Argentum Online
#  Invocado por: make uninstall (desde la raíz del proyecto)
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Configuración (debe ser idéntica a installer.sh)
# ---------------------------------------------------------------------------
APP_NAME="argentum_online"
APP_DISPLAY_NAME="Argentum Online"

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

# Directorios de bases de datos — todos los lugares donde pueden vivir
DB_DIRS_LOCAL=(
    auth_data
    users_data
    worlds
)
DB_DIRS_GLOBAL=(
    "${SHARE_DIR}/auth_data"
    "${SHARE_DIR}/users_data"
    "${SHARE_DIR}/worlds"
    "${CONFIG_DIR}/auth_data"
    "${CONFIG_DIR}/users_data"
    "${CONFIG_DIR}/worlds"
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
# Confirmación interactiva
# ---------------------------------------------------------------------------
echo ""
echo -e "${RED}╔══════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${RED}║            Desinstalador de ${APP_DISPLAY_NAME}                   ${NC}"
echo -e "${RED}╚══════════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo "  Se eliminarán:"
for bin_name in "${APP_BINARIES[@]}"; do
    echo "    - ${BIN_DIR}/${bin_name}  (wrapper)"
done
for desktop_file in "${DESKTOP_FILES[@]}"; do
    echo "    - ${desktop_file}  (ícono de escritorio)"
done
echo "    - ${ICONS_DIR}/  (íconos PNG)"
echo "    - ${SHARE_DIR}/  (binarios reales, assets)"
echo "    - ${CONFIG_DIR}/  (configuración)"
echo ""
echo "  Se preguntará por separado:"
echo "    - auth_data/, users_data/, worlds/  (bases de datos — locales y en el sistema)"
echo "    - build/  (directorio de compilación)"
echo ""
read -r -p "¿Confirmás la desinstalación? [s/N]: " CONFIRM
[[ "$CONFIRM" == "s" || "$CONFIRM" == "S" ]] \
    || { log_warn "Desinstalación cancelada."; exit 0; }
echo ""

# ---------------------------------------------------------------------------
# PASO 1 — Eliminar wrappers de binarios
# ---------------------------------------------------------------------------
log_info "--- Paso 1/5: Eliminando wrappers de binarios ---"

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
    && log_ok "${REMOVED} wrapper(s) eliminado(s)." \
    || log_warn "No se eliminó ningún wrapper (¿ya desinstalado?)."

# ---------------------------------------------------------------------------
# PASO 2 — Eliminar entradas .desktop
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
# PASO 4 — Eliminar binarios reales, assets y configuración
# ---------------------------------------------------------------------------
log_info "--- Paso 4/5: Eliminando binarios, assets y configuración ---"

if [[ -d "${SHARE_DIR}" ]]; then
    rm -rf "${SHARE_DIR}"
    log_ok "Eliminado: ${SHARE_DIR}"
else
    log_warn "No encontrado: ${SHARE_DIR}"
fi

if [[ -d "${CONFIG_DIR}" ]]; then
    rm -rf "${CONFIG_DIR}"
    log_ok "Eliminado: ${CONFIG_DIR}"
else
    log_warn "No encontrado: ${CONFIG_DIR}"
fi

# ---------------------------------------------------------------------------
# PASO 5 — Bases de datos (una sola pregunta, eliminación global + local)
# ---------------------------------------------------------------------------
log_info "--- Paso 5/5: Bases de datos ---"

echo ""
echo -e "  ${YELLOW}Advertencia: las bases de datos contienen todas las cuentas,${NC}"
echo -e "  ${YELLOW}personajes y mundos generados en runtime.${NC}"
echo ""
echo "  Se eliminarían en todos sus ubicaciones:"
echo "    Local  (repo): auth_data/  users_data/  worlds/"
echo "    Global (share): ${SHARE_DIR}/auth_data  users_data  worlds"
echo "    Global (config): ${CONFIG_DIR}/auth_data  users_data  worlds"
echo ""
read -r -p "¿Eliminás las bases de datos? [s/N]: " CLEAN_DB

if [[ "$CLEAN_DB" == "s" || "$CLEAN_DB" == "S" ]]; then
    # Local: dentro del repositorio
    for dir in "${DB_DIRS_LOCAL[@]}"; do
        if [[ -d "$dir" ]]; then
            rm -rf "$dir"
            log_info "  Eliminado (local):  ${dir}/"
        fi
    done

    # Global: en share y config (pueden haber quedado huérfanos si SHARE_DIR
    # ya fue eliminado en el paso 4, en cuyo caso este loop simplemente no
    # encuentra nada; lo dejamos por si el usuario corre solo este paso)
    for dir in "${DB_DIRS_GLOBAL[@]}"; do
        if [[ -d "$dir" ]]; then
            rm -rf "$dir"
            log_info "  Eliminado (global): ${dir}"
        fi
    done

    log_ok "Bases de datos eliminadas (local y global)."
else
    log_info "Bases de datos conservadas."
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
# build/ (pregunta separada)
# ---------------------------------------------------------------------------
if [[ -d "build" ]]; then
    echo ""
    read -r -p "¿Eliminás también el directorio build/? [s/N]: " CLEAN_BUILD
    if [[ "$CLEAN_BUILD" == "s" || "$CLEAN_BUILD" == "S" ]]; then
        rm -rf build/
        log_ok "build/ eliminado."
    else
        log_info "build/ conservado."
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