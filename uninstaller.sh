#!/usr/bin/env bash
# =============================================================================
#  uninstaller.sh — Desinstalador para Argentum Online
#  Uso: bash uninstaller.sh [--name <app_name>] [--purge]
#
#  --purge  elimina también la configuración en ~/.config/NAME/
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Configuración (debe coincidir con installer.sh)
# ---------------------------------------------------------------------------
APP_NAME="${APP_NAME:-argentum_online}"
PURGE=false

BIN_DIR="$HOME/.local/bin"
SHARE_DIR="$HOME/.local/share/${APP_NAME}"
CONFIG_DIR="$HOME/.config/${APP_NAME}"

# Exactamente los binarios que instala installer.sh
APP_BINARIES=(
    argentum_online_server
    argentum_online_client
    argentum_online_editor
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
        --name)  APP_NAME="$2"; shift 2 ;;
        --purge) PURGE=true;    shift   ;;
        *)       die "Argumento desconocido: $1" ;;
    esac
done

SHARE_DIR="$HOME/.local/share/${APP_NAME}"
CONFIG_DIR="$HOME/.config/${APP_NAME}"

# ---------------------------------------------------------------------------
# Confirmación interactiva
# ---------------------------------------------------------------------------
echo ""
echo -e "${RED}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${RED}║          Desinstalador de ${APP_NAME}                        ${NC}"
echo -e "${RED}╚══════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo "  Se eliminarán:"
for bin_name in "${APP_BINARIES[@]}"; do
    echo "    - ${BIN_DIR}/${bin_name}"
done
echo "    - ${SHARE_DIR}/  (maps, resources, game_data)"

if $PURGE; then
    echo "    - ${CONFIG_DIR}/  [--purge activo]"
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
log_info "--- Paso 1/3: Eliminando binarios ---"

REMOVED=0
for bin_name in "${APP_BINARIES[@]}"; do
    target="${BIN_DIR}/${bin_name}"
    if [[ -f "$target" ]]; then
        rm -f "$target"
        log_info "  Eliminado: ${target}"
        REMOVED=$((REMOVED + 1))
    else
        log_warn "  No encontrado (ya eliminado?): ${target}"
    fi
done

[[ $REMOVED -gt 0 ]] \
    && log_ok "${REMOVED} binario(s) eliminado(s)." \
    || log_warn "No se eliminó ningún binario."

# ---------------------------------------------------------------------------
# PASO 2 — Eliminar assets (share)
# ---------------------------------------------------------------------------
log_info "--- Paso 2/3: Eliminando assets ---"

if [[ -d "${SHARE_DIR}" ]]; then
    rm -rf "${SHARE_DIR}"
    log_ok "Eliminado: ${SHARE_DIR}"
else
    log_warn "No encontrado: ${SHARE_DIR}"
fi

# ---------------------------------------------------------------------------
# PASO 3 — Configuración
# ---------------------------------------------------------------------------
log_info "--- Paso 3/3: Configuración ---"

if $PURGE; then
    if [[ -d "${CONFIG_DIR}" ]]; then
        rm -rf "${CONFIG_DIR}"
        log_ok "Eliminado: ${CONFIG_DIR}"
    else
        log_warn "No encontrado: ${CONFIG_DIR}"
    fi
else
    log_info "Configuración conservada en ${CONFIG_DIR}. Usá --purge para eliminarla."
fi

# ---------------------------------------------------------------------------
# Revertir entrada de PATH en el shell RC
# ---------------------------------------------------------------------------
SHELL_RC=""
[[ -f "$HOME/.bashrc" ]] && SHELL_RC="$HOME/.bashrc"
[[ -f "$HOME/.zshrc"  ]] && SHELL_RC="$HOME/.zshrc"

if [[ -n "$SHELL_RC" ]] && grep -q "Agregado por el instalador de ${APP_NAME}" "$SHELL_RC" 2>/dev/null; then
    # Borra el comentario y la línea export que siguen
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
# Bases de datos de runtime (auth_data, users_data, worlds) — sólo con --purge
# ---------------------------------------------------------------------------
if $PURGE; then
    echo ""
    read -r -p "¿Eliminás también las bases de datos locales (auth_data, users_data, worlds)? [s/N]: " CLEAN_DB
    if [[ "$CLEAN_DB" == "s" || "$CLEAN_DB" == "S" ]]; then
        rm -rf auth_data/ users_data/ worlds/
        log_ok "Bases de datos locales eliminadas."
    fi
fi

# ---------------------------------------------------------------------------
# Resumen
# ---------------------------------------------------------------------------
echo ""
echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║   ${APP_NAME} desinstalado correctamente ✓                  ${NC}"
echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
echo ""
