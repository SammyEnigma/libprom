cmake_minimum_required(VERSION 3.9.1)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

execute_process(
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/../VERSION
    OUTPUT_VARIABLE Version
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/../VERSION
    COMMAND cut -f1 -d.
    OUTPUT_VARIABLE MajorVersion
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/../VERSION
    COMMAND cut -f2 -d.
    OUTPUT_VARIABLE MinorVersion
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/../VERSION
    COMMAND cut -f3 -d.
    OUTPUT_VARIABLE PatchVersion
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

project(libpromhttp VERSION ${Version} LANGUAGES C)

set(build_dir ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(public_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(private_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(prom_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../prom/include)
set(public_files ${public_dir}/promhttp.h)
set(private_files ${private_dir}/promhttp.c)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../prom/build /opt/local/lib )

include(FindThreads)

add_library(promhttp SHARED)
set_target_properties(
	promhttp PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(
    promhttp
    PUBLIC ${public_dir} /usr/include /usr/include/microhttpd /opt/local/include ${prom_include_dir}
    PRIVATE ${private_dir}
)

target_sources(
    promhttp
    PUBLIC ${public_files}
    PRIVATE ${private_files}
)

find_library(prom prom HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../prom/build)
find_library(microhttpd microhttpd)

target_compile_options(promhttp PRIVATE "-Werror" "-Wuninitialized" "-Wall" "-Wno-unused-label" "-std=gnu11")
target_compile_options(promhttp PUBLIC "-Werror" "-Wuninitialized" "-Wall" "-Wno-unused-label" "-std=gnu11")

target_link_libraries(promhttp PUBLIC Threads::Threads prom microhttpd)

set(CPACK_PACKAGE_NAME libpromhttp-dev)
set(CPACK_GENERATOR TGZ;DEB)
set(CPACK_PACKAGE_VENDOR DigitalOcean)
set(CPACK_PACKAGE_VERSION_MAJOR ${MajorVersion})
set(CPACK_PACKAGE_VERSION_MINOR ${MinorVersion})
set(CPACK_PACKAGE_VERSION_PATCH ${PatchVersion})
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../README.md)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C based lightweight Server library to expose metrics via HTTP using the Prometheus exposition format")
set(CPACK_PACKAGE_HOMEPAGE_URL https://github.com/jelmd/libprom)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libprom-dev (= ${Version})")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libmicrohttpd-dev")

include(CPack)
include(GNUInstallDirs)
install(TARGETS promhttp LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
