include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

set(test_LIBRARIES
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QJSON_LIBRARY}
    )

# Hackish, but defining dependencies as "DEPENDS buildtests test" does not
# work :(
add_custom_target(check COMMAND ${CMAKE_BUILD_TOOL} buildtests test)

# gtkicontabletest
set(gtkicontabletest_SRCS
    gtkicontabletest.cpp
    ../src/gtkicontable.cpp
    )

kde4_add_unit_test(gtkicontabletest ${gtkicontabletest_SRCS})

target_link_libraries(gtkicontabletest
    ${test_LIBRARIES}
    )
