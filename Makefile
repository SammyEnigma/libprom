CMAKE_EXTRA_OPTS ?= -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_SKIP_BUILD_RPATH=TRUE
MAKE_FLAGS ?= VERBOSE=1

# If TEST is set, build test instead of production binaries
test: TEST := 1
test: TESTDIR := .test

.PHONY: build test clean distclean docs cleandocs

all: build docs

clean:
	rm -rf prom/build
	rm -rf promhttp/build prom/build.test
	rm -rf promtest/build
	cd example && make clean

cleandocs:
	rm -rf docs/html docs/latex

distclean: clean cleandocs
	rm -f vendor/parson/testcpp
	rm -rf bin

buildprom:
	-mkdir prom/build$(TESTDIR) && cd prom/build$(TESTDIR) && \
	TEST=$(TEST) cmake -v -G "Unix Makefiles" $(CMAKE_EXTRA_OPTS) ..
	cd prom/build$(TESTDIR) && make $(MAKE_FLAGS)

# Run "ctest --verbose --force-new-ctest-process" to get the details
test: buildprom
	cd prom/build$(TESTDIR) && make test

buildpromhttp:
	-mkdir promhttp/build && cd promhttp/build && \
	cmake -G "Unix Makefiles" $(CMAKE_EXTRA_OPTS) ..
	cd promhttp/build && $(MAKE) $(MAKE_FLAGS)

build: buildprom buildpromhttp

example:
	cd example && make $(MAKE_FLAGS)

docs: cleandocs
	doxygen Doxyfile

smoke: build
	promtest/prom2json.sh
	-mkdir promtest/build && cd promtest/build && \
	cmake -v -G "Unix Makefiles" $(CMAKE_EXTRA_OPTS) ..
	cd promtest/build && $(MAKE) $(MAKE_FLAGS)
	@echo "Test takes ~ 1 min ..."
	PATH=$${PWD}/bin:$${PATH} \
	LD_LIBRARY_PATH=$${PWD}/prom/build:$${PWD}/promhttp/build \
	promtest/build/promtest
