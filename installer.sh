#!/usr/bin/env bash
# =============================================================================
#  installer.sh — Instalador para Argentum Online
#  Uso: bash installer.sh [--name <app_name>]
#
#  Binarios producidos por el proyecto:
#    argentum_online_server | argentum_online_client | argentum_online_editor
#  Tests:
#    argentum_online_tests
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Configuración
# ---------------------------------------------------------------------------
APP_NAME="${APP_NAME:-argentum_online}"
BUILD_DIR="build"
JOBS=$(nproc 2>/dev/null || echo 4)

BIN_DIR="$HOME/.local/bin"
SHARE_DIR="$HOME/.local/share/${APP_NAME}"
CONFIG_DIR="$HOME/.config/${APP_NAME}"

# Binarios de la aplicación (excluye el de tests)
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
        --name) APP_NAME="$2"; shift 2 ;;
        *)      die "Argumento desconocido: $1" ;;
    esac
done

# Recalcular rutas si se sobreescribió APP_NAME
SHARE_DIR="$HOME/.local/share/${APP_NAME}"
CONFIG_DIR="$HOME/.config/${APP_NAME}"

# ---------------------------------------------------------------------------
# Verificaciones previas
# ---------------------------------------------------------------------------
log_info "=== Iniciando instalación de ${APP_NAME} ==="

[[ -f "Makefile" ]] \
    || die "Ejecutá el instalador desde la raíz del proyecto (donde está el Makefile)."

command -v cmake  &>/dev/null || die "cmake no encontrado.  sudo apt install cmake"
command -v make   &>/dev/null || die "make no encontrado.   sudo apt install build-essential"

# ---------------------------------------------------------------------------
# PASO 1 — Dependencias del sistema
# ---------------------------------------------------------------------------
log_info "--- Paso 1/5: Instalando dependencias del sistema ---"

PACKAGES=(
    build-essential
    cmake
    libsdl2-dev
    libsdl2-image-dev
    libsdl2-mixer-dev
    libsdl2-ttf-dev
    libsdl2-net-dev
    libyaml-cpp-dev
    libgtest-dev
    pkg-config
    valgrind
)

if command -v apt-get &>/dev/null; then
    log_info "Actualizando lista de paquetes..."
    sudo apt-get update -qq

    MISSING=()
    for pkg in "${PACKAGES[@]}"; do
        dpkg -s "$pkg" &>/dev/null || MISSING+=("$pkg")
    done

    if [[ ${#MISSING[@]} -gt 0 ]]; then
        log_info "Instalando paquetes faltantes: ${MISSING[*]}"
        sudo apt-get install -y "${MISSING[@]}"
    else
        log_ok "Todas las dependencias ya están instaladas."
    fi
else
    log_warn "apt-get no disponible. Verificá manualmente que SDL2 y dependencias estén instaladas."
fi

log_ok "Dependencias OK."

# ---------------------------------------------------------------------------
# PASO 2 — Compilar (equivalente a 'make compile-debug')
# ---------------------------------------------------------------------------
log_info "--- Paso 2/5: Compilando el proyecto ---"

# El Makefile también crea symlinks y directorios de datos; los replicamos aquí
mkdir -p auth_data users_data worlds
mkdir -p "${BUILD_DIR}"

cmake -S . -B "./${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug
cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

# Symlinks que el Makefile establece para que los binarios encuentren sus recursos
ln -sfn ../maps        "${BUILD_DIR}/maps"
ln -sfn ../config      "${BUILD_DIR}/config"
ln -sfn ../auth_data   "${BUILD_DIR}/auth_data"
ln -sfn ../users_data  "${BUILD_DIR}/users_data"
ln -sfn ../worlds      "${BUILD_DIR}/worlds"
ln -sfn ../resources   "${BUILD_DIR}/resources"

log_ok "Compilación exitosa."

# ---------------------------------------------------------------------------
# PASO 3 — Tests unitarios (equivalente a 'make run-tests')
# ---------------------------------------------------------------------------
log_info "--- Paso 3/5: Ejecutando tests unitarios ---"

TEST_BIN="${BUILD_DIR}/argentum_online_tests"

[[ -f "$TEST_BIN" ]] \
    || die "El ejecutable de tests no fue encontrado en ${TEST_BIN}. Revisá la compilación."

(cd "${BUILD_DIR}" && ./argentum_online_tests) \
    || die "Los tests fallaron. Abortando instalación."

log_ok "Todos los tests pasaron."

# ---------------------------------------------------------------------------
# PASO 4 — Crear estructura de directorios de destino
# ---------------------------------------------------------------------------
log_info "--- Paso 4/5: Preparando directorios de instalación ---"

mkdir -p "${BIN_DIR}"

# Assets
mkdir -p "${SHARE_DIR}/maps"
mkdir -p "${SHARE_DIR}/resources"
mkdir -p "${SHARE_DIR}/game_data"
mkdir -p "${SHARE_DIR}/worlds"       # directorio vacío inicial (datos en runtime)
mkdir -p "${SHARE_DIR}/auth_data"    # ídem
mkdir -p "${SHARE_DIR}/users_data"   # ídem

# Configuración
mkdir -p "${CONFIG_DIR}"

log_ok "Directorios creados."

# ---------------------------------------------------------------------------
# PASO 5 — Copiar binarios, assets y configuración
# ---------------------------------------------------------------------------
log_info "--- Paso 5/5: Copiando archivos ---"

# ---- 5a. Binarios ----
INSTALLED_BINS=0
for bin_name in "${APP_BINARIES[@]}"; do
    bin_path="${BUILD_DIR}/${bin_name}"
    if [[ -f "$bin_path" ]]; then
        log_info "  Binario: ${bin_name} → ${BIN_DIR}/"
        install -m 755 "$bin_path" "${BIN_DIR}/"
        INSTALLED_BINS=$((INSTALLED_BINS + 1))
    else
        log_warn "  Binario no encontrado (¿target deshabilitado en CMake?): ${bin_path}"
    fi
done

[[ $INSTALLED_BINS -gt 0 ]] \
    || die "No se instaló ningún binario. Verificá que CMake esté generando los targets correctamente."

log_ok "${INSTALLED_BINS} binario(s) instalado(s) en ${BIN_DIR}."

# ---- 5b. Assets ----
# maps/
if [[ -d "maps" ]]; then
    cp -r maps/. "${SHARE_DIR}/maps/"
    log_info "  Assets: maps/ → ${SHARE_DIR}/maps/"
fi

# resources/ (imágenes, audio, fuentes, etc.)
if [[ -d "resources" ]]; then
    cp -r resources/. "${SHARE_DIR}/resources/"
    log_info "  Assets: resources/ → ${SHARE_DIR}/resources/"
fi

# game_data/ (si existe)
if [[ -d "game_data" ]]; then
    cp -r game_data/. "${SHARE_DIR}/game_data/"
    log_info "  Assets: game_data/ → ${SHARE_DIR}/game_data/"
fi

log_ok "Assets instalados en ${SHARE_DIR}."

# ---- 5c. Configuración ----
if [[ -d "config" ]]; then
    cp -r config/. "${CONFIG_DIR}/"
    log_info "  Config: config/ → ${CONFIG_DIR}/"
fi

log_ok "Configuración instalada en ${CONFIG_DIR}."

# ---------------------------------------------------------------------------
# Asegurar ~/.local/bin en el PATH
# ---------------------------------------------------------------------------
SHELL_RC=""
[[ -f "$HOME/.bashrc" ]] && SHELL_RC="$HOME/.bashrc"
[[ -f "$HOME/.zshrc"  ]] && SHELL_RC="$HOME/.zshrc"

if [[ -n "$SHELL_RC" ]] && ! grep -q 'HOME/.local/bin' "$SHELL_RC" 2>/dev/null; then
    {
        echo ''
        echo "# Agregado por el instalador de ${APP_NAME}"
        echo 'export PATH="$HOME/.local/bin:$PATH"'
    } >> "$SHELL_RC"
    log_info "~/.local/bin agregado al PATH en ${SHELL_RC}."
    log_warn "Reiniciá tu terminal o ejecutá: source ${SHELL_RC}"
fi

# ---------------------------------------------------------------------------
# Resumen
# ---------------------------------------------------------------------------
echo ""
echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║   ${APP_NAME} instalado correctamente ✓                     ${NC}"
echo -e "${GREEN}╠══════════════════════════════════════════════════════════════╣${NC}"
printf "${GREEN}║${NC}  Binarios  →  %s\n" "${BIN_DIR}/"
printf "${GREEN}║${NC}  Assets    →  %s\n" "${SHARE_DIR}/"
printf "${GREEN}║${NC}  Config    →  %s\n" "${CONFIG_DIR}/"
echo -e "${GREEN}╠══════════════════════════════════════════════════════════════╣${NC}"
echo -e "${GREEN}║${NC}  Servidor : argentum_online_server <puerto> --load <mundo>"
echo -e "${GREEN}║${NC}  Cliente  : argentum_online_client"
echo -e "${GREEN}║${NC}  Editor   : argentum_online_editor"
echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
echo ""
