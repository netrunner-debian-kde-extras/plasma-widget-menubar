project(plasma-widget-menubar)

cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

# Find the required libraries
find_package(KDE4 4.4 REQUIRED)
include(KDE4Defaults)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUSMENUQT REQUIRED dbusmenu-qt>=0.6.0)

find_package(QJson REQUIRED)

include_directories(
    ${KDE4_INCLUDES}
    ${DBUSMENUQT_INCLUDE_DIRS}
    ${QJSON_INCLUDE_DIR}
    )

link_directories(
    ${DBUSMENUQT_LIBRARY_DIRS}
    )

add_subdirectory(src)
#add_subdirectory(tests)

# Packaging
set(PROJECT_VERSION "0.2.0")
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target(dist
    COMMAND bzr export --root=${ARCHIVE_NAME} ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_target(distcheck
    COMMAND cd ${CMAKE_BINARY_DIR}
    && rm -rf ${ARCHIVE_NAME}
    && tar xf ${ARCHIVE_NAME}.tar.bz2
    && mkdir ${ARCHIVE_NAME}/build
    && cd ${ARCHIVE_NAME}/build
    && cmake -DCMAKE_INSTALL_PREFIX=../install ..
    && make
    && make install
    && make check
    )
add_dependencies(distcheck dist)
