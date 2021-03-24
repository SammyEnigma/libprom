CMAKE_EXTRA_OPTS ?= -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_SKIP_BUILD_RPATH=TRUE
MAKE_FLAGS ?= VERBOSE=1
CFLAGS ?= -DPROM_LOG_ENABLE -g -O3

LIB_PATH_SFX := $(shell [ `uname -s` = 'SunOS' ] && echo  '_64' )
LIB_PATH := .:$$PWD/prom/build:$$PWD/promhttp/build:/opt/local/lib

# If TEST is set, build test instead of production binaries
test: TEST := 1
test: TESTDIR := .test
test: CFLAGS += -Og

# Enable troubleshooting info per default.
prom: CMAKE_EXTRA_OPTS += -DCMAKE_C_FLAGS="$(CFLAGS)"

.PHONY: build test clean distclean docs cleandocs prom promhttp example

all: build

clean:
	rm -rf prom/build prom/build.test
	rm -rf promhttp/build
	rm -rf promtest/build
	cd example && $(MAKE) clean

cleandocs:
	rm -rf docs/html docs/latex Doxyfile.tmp

distclean: clean cleandocs
	rm -f vendor/parson/testcpp
	rm -rf bin

prom:
	-mkdir prom/build$(TESTDIR) && cd prom/build$(TESTDIR) && \
	TEST=$(TEST) cmake -v -G "Unix Makefiles" $(CMAKE_EXTRA_OPTS) ..
	cd prom/build$(TESTDIR) && $(MAKE) $(MAKE_FLAGS)

# Run "ctest --verbose --force-new-ctest-process" to get the details
test: prom
	cd prom/build$(TESTDIR) && LD_LIBRARY_PATH$(LIB_PATH_SFX)=$(LIB_PATH) \
	$(MAKE) test

promhttp:
	-mkdir promhttp/build && cd promhttp/build && \
	cmake -G "Unix Makefiles" $(CMAKE_EXTRA_OPTS) ..
	cd promhttp/build && $(MAKE) $(MAKE_FLAGS)

build: prom promhttp

example: build
	cd example && $(MAKE) $(MAKE_FLAGS)

docs: cleandocs
	VERS=$$( cat VERSION ) && \
		sed -e "s|@VERSION@|$$VERS|" Doxyfile >Doxyfile.tmp
	doxygen Doxyfile.tmp

# If this test fails with something like 'JSON: root container is not an []'
# the prom2json version picked up may not work, or is possibly for the wrong
# architecture etc..  Check promtest/prom2json.sh for more information.
smoke: build
	promtest/prom2json.sh
	-mkdir promtest/build && cd promtest/build && \
	cmake -v -G "Unix Makefiles" $(CMAKE_EXTRA_OPTS) ..
	cd promtest/build && $(MAKE) $(MAKE_FLAGS)
	@echo "Test takes ~ 1 min ..."
	PATH=$${PWD}/bin:$${PATH} \
	LD_LIBRARY_PATH$(LIB_PATH_SFX)=$(LIB_PATH):$$PWD/promtest/build \
	promtest/build/promtest
