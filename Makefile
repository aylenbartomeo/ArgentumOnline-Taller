.PHONY: all test clean editor client common server compile-debug prepare-dirs run-tests valgrind-tests compile-and-tests valgrind-server valgrind-stop-server valgrind-show install uninstall run-server-create run-server-load run-client run-editor clean-db-auth clean-db-world clean-db check-config

default: all

# ─── Configuración Dinámica mediante TOML ──────
CONFIG_FILE := config/config.toml

RAW_PORT  := $(shell python3 -c "b=open('$(CONFIG_FILE)').read().split('[server]')[1].split('[')[0]; p=[l.split('=')[1].strip().strip('\"').strip('\'') for l in b.splitlines() if l.strip().startswith('port')]; print(p[0] if p else '')" 2>/dev/null)
RAW_WORLD := $(shell python3 -c "b=open('$(CONFIG_FILE)').read().split('[server]')[1].split('[')[0]; w=[l.split('=')[1].strip().strip('\"').strip('\'') for l in b.splitlines() if l.strip().startswith('world')]; print(w[0] if w else '')" 2>/dev/null)
RAW_MAP   := $(shell python3 -c "b=open('$(CONFIG_FILE)').read().split('[server]')[1].split('[')[0]; m=[l.split('=')[1].strip().strip('\"').strip('\'') for l in b.splitlines() if l.strip().startswith('map')]; print(m[0] if m else '')" 2>/dev/null)

# Control de Vacío Estricto: Si la extracción falló, forzamos los valores tradicionales.
ifeq ($(strip $(RAW_PORT)),)
    PORT := 8080
else
    PORT := $(RAW_PORT)
endif

ifeq ($(strip $(RAW_WORLD)),)
    WORLD := DefaultWorld
else
    WORLD := $(RAW_WORLD)
endif

ifeq ($(strip $(RAW_MAP)),)
    MAP := maps/defaultMap.json
else
    MAP := $(RAW_MAP)
endif

# ─── Validación Preventiva ────────────────────────────────────────────────────
check-config:
	@if [ ! -f $(CONFIG_FILE) ]; then \
		echo "[WARN] No se encontró '$(CONFIG_FILE)'. Usando valores por defecto (?=)."; \
	fi

# ─── Compilación y Tests ─────────────────────
compile-debug:
	mkdir -p build/
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug $(EXTRA_GENERATE)
	cmake --build build/ $(EXTRA_COMPILE)
	@# --- Recursos del Servidor ---
	@mkdir -p auth_data users_data worlds
	@ln -sfn ../maps build/maps
	@ln -sfn ../config build/config
	@ln -sfn ../auth_data build/auth_data
	@ln -sfn ../users_data build/users_data
	@ln -sfn ../worlds build/worlds
	@# --- Recursos del Cliente ---
	@ln -sfn ../resources build/resources

prepare-dirs:
	@mkdir -p auth_data users_data worlds

run-tests: prepare-dirs
	cd build && ./argentum_online_tests

valgrind-tests: prepare-dirs
	mkdir -p build/valgrind
	cd build && valgrind --leak-check=full \
	                     --show-leak-kinds=all \
	                     --track-origins=yes \
	                     --log-file=valgrind/reporte_tests.log \
	                     ./argentum_online_tests
	@echo "Chequeo de Valgrind completado. Reporte guardado en: build/valgrind/reporte_tests.log"

compile-and-tests: compile-debug
	cd build && ./argentum_online_tests

valgrind-server: check-config prepare-dirs
	mkdir -p build/valgrind
	cd build && valgrind --leak-check=full \
	                     --show-leak-kinds=all \
	                     --track-origins=yes \
	                     --log-file=valgrind/reporte_server.log \
	                     ./argentum_online_server $(PORT) --load "$(WORLD)" & \
	echo $$! > build/valgrind/server.pid
	@echo "Servidor bajo Valgrind corriendo (PID guardado en build/valgrind/server.pid)"
	@echo "Jugá la partida, luego ejecutá: make valgrind-stop-server"

valgrind-stop-server:
	@PID=$$(cat build/valgrind/server.pid 2>/dev/null) && \
	if [ -n "$$PID" ]; then \
		kill -TERM $$PID && echo "Servidor detenido (PID $$PID). Esperando reporte..."; \
		sleep 2; \
		rm -f build/valgrind/server.pid; \
	else \
		echo "[WARN] No se encontró PID guardado."; \
	fi
	@echo "Reporte en: build/valgrind/reporte_server.log"

valgrind-show:
	@echo "=== REPORTE TESTS ===" && cat build/valgrind/reporte_tests.log 2>/dev/null || echo "(no existe)"
	@echo "=== REPORTE SERVER ===" && cat build/valgrind/reporte_server.log 2>/dev/null || echo "(no existe)"

all: clean run-tests

clean:
	rm -Rf build/


# ─── Ejecución de Binarios (Modificados para usar las variables TOML) ──────────

run-server-create: check-config prepare-dirs
	cd build && ./argentum_online_server $(PORT) --create "$(WORLD)" --map "$(MAP)"

run-server-load: check-config prepare-dirs
	cd build && ./argentum_online_server $(PORT) --load "$(WORLD)"

run-client: prepare-dirs
	cd build && ./argentum_online_client $(ARGS)

run-editor: prepare-dirs
	cd build && ./argentum_online_editor

clean-db-auth:
	rm -rf auth_data/ users_data/ build/auth_data/ build/users_data/
	@echo "Base de datos de cuentas y usuarios (auth_data, users_data) limpiada."

clean-db-world:
	rm -rf worlds/ build/worlds/
	@echo "Base de datos de mundos (worlds) limpiada."

clean-db: clean-db-auth clean-db-world
	@echo "Base de datos completa limpiada correctamente."


# ─── Instalación / Desinstalación Global ──────────────────────────────────────
install:
	@chmod +x ./installer.sh
	@./installer.sh

uninstall:
	@chmod +x ./uninstaller.sh
	@./uninstaller.sh --purge
