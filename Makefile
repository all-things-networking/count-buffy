.PHONY: all install-deps build init

all: build

clean:
	rm -rf build

init:
	conan profile detect --force

install-deps: init
	conan install . --build=missing -s compiler.cppstd=20
	cp build/Release/generators/CMakePresets.json ./CMakeUserPresets.json

config: install-deps
	cmake --preset conan-release

build: config
	cmake --build --preset conan-release --parallel