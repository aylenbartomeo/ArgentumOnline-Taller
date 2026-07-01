#!/usr/bin/env bash
# =============================================================================
#  installer.sh — Instalador para Argentum Online
#  Invocado por: make install (desde la raíz del proyecto)
#
#  Binarios: argentum_online_server | argentum_online_client | argentum_online_editor
#  Tests:    argentum_online_tests
#  Íconos:   resources/ui/logos/clientLogo.png | editorLogo.png
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Configuración
# ---------------------------------------------------------------------------
APP_NAME="argentum_online"
APP_DISPLAY_NAME="Argentum Online"
BUILD_DIR="build"
JOBS=$(nproc 2>/dev/null || echo 4)

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
# Verificaciones previas
# ---------------------------------------------------------------------------
log_info "=== Iniciando instalación de ${APP_DISPLAY_NAME} ==="

[[ -f "Makefile" ]] \
    || die "Ejecutá 'make install' desde la raíz del proyecto (donde está el Makefile)."

# ---------------------------------------------------------------------------
# PASO 1 — Dependencias del sistema
# ---------------------------------------------------------------------------
log_info "--- Paso 1/5: Instalando dependencias del sistema ---"

PACKAGES=(
    build-essential
    cmake
    git                 # Asegura control de versiones si se usa en VM limpia
    pkg-config
    valgrind
    python3
    # --- Herramientas de Interfaz y Lanzador (Launcher) ---
    qtbase5-dev
    qtbase5-dev-tools
    # --- Librerías de desarrollo nativas de SDL2 ---
    libsdl2-dev
    libsdl2-image-dev
    libsdl2-mixer-dev
    libsdl2-ttf-dev
    libsdl2-net-dev
    # --- Códecs y Extensiones de Audio/Fuentes ---
    libopus-dev
    libopusfile-dev
    libxmp-dev
    fluidsynth
    libfluidsynth-dev
    libwavpack1
    libwavpack-dev
    wavpack
    libfreetype-dev
    # --- Otras dependencias del motor ---
    libyaml-cpp-dev
    libgtest-dev
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
# PASO 2 — Compilar
# ---------------------------------------------------------------------------
log_info "--- Paso 2/5: Compilando el proyecto ---"

mkdir -p auth_data users_data worlds
mkdir -p "${BUILD_DIR}"

cmake -S . -B "./${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

# Symlinks para que los binarios encuentren sus recursos al ejecutarse desde build/
ln -sfn ../maps       "${BUILD_DIR}/maps"
ln -sfn ../config     "${BUILD_DIR}/config"
ln -sfn ../auth_data  "${BUILD_DIR}/auth_data"
ln -sfn ../users_data "${BUILD_DIR}/users_data"
ln -sfn ../worlds     "${BUILD_DIR}/worlds"
ln -sfn ../resources  "${BUILD_DIR}/resources"

log_ok "Compilación exitosa."

# ---------------------------------------------------------------------------
# PASO 3 — Tests unitarios
# ---------------------------------------------------------------------------
log_info "--- Paso 3/5: Ejecutando tests unitarios ---"

TEST_BIN="${BUILD_DIR}/argentum_online_tests"

[[ -f "$TEST_BIN" ]] \
    || die "Ejecutable de tests no encontrado en ${TEST_BIN}. Revisá la compilación."

(cd "${BUILD_DIR}" && ./argentum_online_tests) \
    || die "Los tests fallaron. Abortando instalación."

log_ok "Todos los tests pasaron."

# ---------------------------------------------------------------------------
# PASO 4 — Crear estructura de directorios de destino
# ---------------------------------------------------------------------------
log_info "--- Paso 4/5: Preparando directorios de instalación ---"

mkdir -p "${BIN_DIR}"
mkdir -p "${SHARE_DIR}/maps"
mkdir -p "${SHARE_DIR}/resources"
mkdir -p "${SHARE_DIR}/game_data"
mkdir -p "${SHARE_DIR}/worlds"       # runtime: el servidor escribe acá
mkdir -p "${SHARE_DIR}/auth_data"    # runtime: cuentas de usuario
mkdir -p "${SHARE_DIR}/users_data"   # runtime: datos de personajes
mkdir -p "${CONFIG_DIR}"
mkdir -p "${ICONS_DIR}"
mkdir -p "${DESKTOP_DIR}"

log_ok "Directorios creados."

# ---------------------------------------------------------------------------
# PASO 5 — Copiar binarios, assets, config e íconos; crear .desktop
# ---------------------------------------------------------------------------
log_info "--- Paso 5/5: Instalando archivos ---"

# ── 5a. Binarios e Instalación de Wrappers Globales ─────────────────────────
INSTALLED_BINS=0
for bin_name in "${APP_BINARIES[@]}"; do
    bin_path="${BUILD_DIR}/${bin_name}"
    if [[ -f "$bin_path" ]]; then
        # Instalamos el binario real dentro del directorio share
        install -m 755 "$bin_path" "${SHARE_DIR}/"
        
        # Creamos un script ejecutable wrapper en el BIN_DIR del usuario
        cat > "${BIN_DIR}/${bin_name}" << EOF
#!/usr/bin/env bash
# Wrapper dinámico generado por el instalador de Argentum Online
cd "${SHARE_DIR}" && exec "./${bin_name}" "\$@"
EOF
        chmod 755 "${BIN_DIR}/${bin_name}"
        
        log_info "  Binario: ${bin_name} → ${SHARE_DIR}/ (Wrapper en ${BIN_DIR}/)"
        INSTALLED_BINS=$((INSTALLED_BINS + 1))
    else
        log_warn "  Binario no encontrado (¿target deshabilitado en CMake?): ${bin_path}"
    fi
done

[[ $INSTALLED_BINS -gt 0 ]] \
    || die "No se instaló ningún binario. Verificá los targets de CMake."

log_ok "${INSTALLED_BINS} binario(s) y wrapper(s) configurado(s) globalmente."

# ── 5b. Assets ──────────────────────────────────────────────────────────────
for asset_dir in maps resources game_data; do
    if [[ -d "$asset_dir" ]]; then
        cp -r "${asset_dir}/." "${SHARE_DIR}/${asset_dir}/"
        log_info "  Assets: ${asset_dir}/ → ${SHARE_DIR}/${asset_dir}/"
    fi
done

log_ok "Assets instalados en ${SHARE_DIR}."

# ── 5c. Configuración ───────────────────────────────────────────────────────
if [[ -d "config" ]]; then
    cp -r config/. "${CONFIG_DIR}/"
    log_info "  Config: config/ → ${CONFIG_DIR}/"
    
    # Enlace simbólico relativo dentro de SHARE_DIR para que los binarios localicen la config local
    ln -sfn "${CONFIG_DIR}" "${SHARE_DIR}/config"
    log_info "  Symlink de compatibilidad: ${SHARE_DIR}/config → ${CONFIG_DIR}"
fi

log_ok "Configuración instalada en ${CONFIG_DIR}."

# ── 5d. Íconos ──────────────────────────────────────────────────────────────
ICONS_DIR="$HOME/.local/share/icons"
mkdir -p "${ICONS_DIR}"

CLIENT_ICON_SRC="resources/ui/logos/clientLogo.png"
EDITOR_ICON_SRC="resources/ui/logos/editorLogo.png"

CLIENT_ICON_DST="${ICONS_DIR}/clientLogo.png"
EDITOR_ICON_DST="${ICONS_DIR}/editorLogo.png"

if [[ -f "$CLIENT_ICON_SRC" ]]; then
    cp "$CLIENT_ICON_SRC" "$CLIENT_ICON_DST"
    log_info "  Ícono cliente → ${CLIENT_ICON_DST}"
else
    log_warn "  Ícono cliente no encontrado: ${CLIENT_ICON_SRC}"
fi

if [[ -f "$EDITOR_ICON_SRC" ]]; then
    cp "$EDITOR_ICON_SRC" "$EDITOR_ICON_DST"
    log_info "  Ícono editor  → ${EDITOR_ICON_DST}"
else
    log_warn "  Ícono editor no encontrado: ${EDITOR_ICON_SRC}"
fi

# ── 5e. Entradas .desktop (íconos de escritorio) ────────────────────────────
CLIENT_DESKTOP="${DESKTOP_DIR}/argentum_online_client.desktop"
EDITOR_DESKTOP="${DESKTOP_DIR}/argentum_online_editor.desktop"

cat > "$CLIENT_DESKTOP" << DESKTOP
[Desktop Entry]
Version=1.0
Type=Application
Name=Argentum Online
GenericName=Cliente de Argentum Online
Comment=Conectate al mundo de Argentum Online
Exec=${BIN_DIR}/argentum_online_client --fullscreen
Icon=${CLIENT_ICON_DST}
Path=${SHARE_DIR}
Terminal=false
Categories=Game;RolePlaying;
Keywords=argentum;online;rpg;mmo;
StartupNotify=true
StartupWMClass=argentum_online_client
DESKTOP

cat > "$EDITOR_DESKTOP" << DESKTOP
[Desktop Entry]
Version=1.0
Type=Application
Name=Argentum Online Editor
GenericName=Editor de mapas de Argentum Online
Comment=Creá y editá mundos de Argentum Online
Exec=${BIN_DIR}/argentum_online_editor --fullscreen
Icon=${EDITOR_ICON_DST}
Path=${SHARE_DIR}
Terminal=false
Categories=Game;RolePlaying;
Keywords=argentum;editor;mapas;
StartupNotify=true
StartupWMClass=argentum_online_editor
DESKTOP

chmod 644 "$CLIENT_DESKTOP" "$EDITOR_DESKTOP"
log_info "  .desktop: argentum_online_client.desktop → ${DESKTOP_DIR}/"
log_info "  .desktop: argentum_online_editor.desktop → ${DESKTOP_DIR}/"

if command -v update-desktop-database &>/dev/null; then
    update-desktop-database "${DESKTOP_DIR}" 2>/dev/null || true
fi

if command -v gtk-update-icon-cache &>/dev/null; then
    gtk-update-icon-cache -f -t "${ICONS_DIR}" 2>/dev/null || true
fi

log_ok "Íconos de escritorio registrados y mapeados con el entorno gráfico."

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
# Resumen final
# ---------------------------------------------------------------------------
echo ""
echo -e "${GREEN}╔══════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║        ${APP_DISPLAY_NAME} instalado correctamente ✓              ${NC}"
echo -e "${GREEN}╠══════════════════════════════════════════════════════════════════╣${NC}"
printf "${GREEN}║${NC}  Wrappers Globales →  %s\n"   "${BIN_DIR}/"
printf "${GREEN}║${NC}  Binarios y Assets →  %s\n"   "${SHARE_DIR}/"
printf "${GREEN}║${NC}  Configuración     →  %s\n"   "${CONFIG_DIR}/"
printf "${GREEN}║${NC}  Íconos            →  %s\n"   "${ICONS_DIR}/"
printf "${GREEN}║${NC}  Escritorio        →  %s\n"   "${DESKTOP_DIR}/"
echo -e "${GREEN}╠══════════════════════════════════════════════════════════════════╣${NC}"
echo ""