.PHONY: all test clean editor client common server compile-debug run-tests valgrind-tests

default: all

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

run-tests:
	cd build && ./argentum_online_tests

valgrind-tests:
	mkdir -p build/valgrind
	cd build && valgrind --leak-check=full \
	                     --show-leak-kinds=all \
	                     --track-origins=yes \
	                     --log-file=valgrind/reporte_tests.log \
	                     ./argentum_online_tests
	@echo "Chequeo de Valgrind completado. Reporte guardado en: build/valgrind/reporte_tests.log"

compile-and-tests: compile-debug
	cd build && ./argentum_online_tests

PORT ?= 8080
WORLD ?= DefaultWorld
MAP ?= maps/defaultMap.json

prepare-dirs:
	@mkdir -p auth_data users_data worlds

run-server-create: prepare-dirs
	cd build && ./argentum_online_server $(PORT) --create "$(WORLD)" --map "$(MAP)"

run-server-load: prepare-dirs
	cd build && ./argentum_online_server $(PORT) --load "$(WORLD)"

run-client:
	cd build && ./argentum_online_client

run-editor:
	cd build && ./argentum_online_editor

all: clean run-tests

clean:
	rm -Rf build/

clean-db-auth:
	rm -rf auth_data/ users_data/ build/auth_data/ build/users_data/
	@echo "Base de datos de cuentas y usuarios (auth_data, users_data) limpiada."

clean-db-world:
	rm -rf worlds/ build/worlds/
	@echo "Base de datos de mundos (worlds) limpiada."

clean-db: clean-db-auth clean-db-world
	@echo "Base de datos completa limpiada correctamente."
