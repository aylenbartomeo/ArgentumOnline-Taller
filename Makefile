.PHONY: all test clean editor client common server compile-debug run-tests valgrind-tests

default: all

compile-debug:
	mkdir -p build/
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug $(EXTRA_GENERATE)
	cmake --build build/ $(EXTRA_COMPILE)
	@# --- Recursos del Servidor ---
	@ln -sfn ../maps build/maps
	@ln -sfn ../config build/config
	@ln -sfn ../auth_data build/auth_data
	@ln -sfn ../users_data build/users_data
	@# --- Recursos del Cliente ---
	@ln -sfn ../resources build/resources
run-tests: compile-debug
	cd build && ./tests/argentum_online_tests

valgrind-tests: compile-debug
	mkdir -p build/valgrind
	cd build && valgrind --leak-check=full \
	                     --show-leak-kinds=all \
	                     --track-origins=yes \
	                     --log-file=valgrind/reporte_tests.log \
	                     ./argentum_online_tests
	@echo "Chequeo de Valgrind completado. Reporte guardado en: build/valgrind/reporte_tests.log"

PORT ?= 8080
run-server:
	cd build && ./argentum_online_server $(PORT)

run-client:
	cd build && ./argentum_online_client

run-editor:
	cd build && ./argentum_online_editor

all: clean run-tests

clean:
	rm -Rf build/
