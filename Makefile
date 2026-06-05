.PHONY: all test clean editor client common server compile-debug run-tests valgrind-tests

default: all

compile-debug:
	mkdir -p build/
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug $(EXTRA_GENERATE)
	cmake --build build/ $(EXTRA_COMPILE)

run-tests: compile-debug
	cd build && ./tests/argentum_online_tests

valgrind-tests: compile-debug
	cd build && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./tests/argentum_online_tests

all: clean run-tests

clean:
	rm -Rf build/
