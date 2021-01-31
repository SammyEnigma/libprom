CMAKE_EXTRA_OPTS ?= -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_SKIP_BUILD_RPATH=TRUE
MAKE_FLAGS ?= VERBOSE=1
CFLAGS ?= -DPROM_LOG_ENABLE

# If TEST is set, build test instead of production binaries
test: TEST := 1
test: TESTDIR := .test

# Enable troubleshooting info per default.
prom: CMAKE_EXTRA_OPTS += -DCMAKE_C_FLAGS="-DPROM_LOG_ENABLE"

.PHONY: build test clean distclean docs cleandocs prom promhttp example

all: build docs

clean:
	rm -rf prom/build prom/build.test
	rm -rf promhttp/build
	rm -rf promtest/build
	cd example && make clean

cleandocs:
	rm -rf docs/html docs/latex Doxyfile.tmp

distclean: clean cleandocs
	rm -f vendor/parson/testcpp
	rm -rf bin

prom:
	-mkdir prom/build$(TESTDIR) && cd prom/build$(TESTDIR) && \
	TEST=$(TEST) cmake -v -G "Unix Makefiles" $(CMAKE_EXTRA_OPTS) ..
	cd prom/build$(TESTDIR) && make $(MAKE_FLAGS)

# Run "ctest --verbose --force-new-ctest-process" to get the details
test: prom
	cd prom/build$(TESTDIR) && make test

promhttp:
	-mkdir promhttp/build && cd promhttp/build && \
	cmake -G "Unix Makefiles" $(CMAKE_EXTRA_OPTS) ..
	cd promhttp/build && $(MAKE) $(MAKE_FLAGS)

build: prom promhttp

example:
	cd example && make $(MAKE_FLAGS)

docs: cleandocs
	VERS=$$( cat VERSION ) && \
		sed -e "s|@VERSION@|$$VERS|" Doxyfile >Doxyfile.tmp
	doxygen Doxyfile.tmp

smoke: build
	promtest/prom2json.sh
	-mkdir promtest/build && cd promtest/build && \
	cmake -v -G "Unix Makefiles" $(CMAKE_EXTRA_OPTS) ..
	cd promtest/build && $(MAKE) $(MAKE_FLAGS)
	@echo "Test takes ~ 1 min ..."
	PATH=$${PWD}/bin:$${PATH} \
	LD_LIBRARY_PATH=$${PWD}/prom/build:$${PWD}/promhttp/build \
	promtest/build/promtest
