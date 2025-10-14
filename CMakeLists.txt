option(ASSERTIONS_INSTALL "Generate the install target." ${PROJECT_IS_TOP_LEVEL})
option(ASSERTIONS_DEVELOPMENT_MODE "Include additional targets for development" OFF)
option(ASSERTIONS_CCACHE "Use ccache" OFF)

if (ASSERTIONS_DEVELOPMENT_MODE)
    cmake_minimum_required(VERSION 3.25)
    if (ASSERTIONS_CCACHE)
        include(cmake/ccache.cmake)
        enable_ccache()
    endif()
else()
    cmake_minimum_required(VERSION 3.21)
endif()

project(assertions)

add_library(assertions INTERFACE)
add_library(assertions::assertions ALIAS assertions)
target_include_directories(assertions INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(NOT CMAKE_SKIP_INSTALL_RULES AND ASSERTIONS_INSTALL)
    include(GNUInstallDirs)

    install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
    )

    install(
        EXPORT ${PROJECT_NAME}Targets
        NAMESPACE ${PROJECT_NAME}::
        FILE ${PROJECT_NAME}Config.cmake
        DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
    )

    install(
        DIRECTORY "${PROJECT_SOURCE_DIR}/include/assertions"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h"
    )
endif()

if(ASSERTIONS_DEVELOPMENT_MODE)
    enable_testing()
    include(FetchContent)
    include(cmake/clang-format.cmake)
    if(WIN32)
        include(cmake/group-sources.cmake)
    endif()
    include(cmake/md.cmake)
    include(cmake/sanitizers.cmake)

    set(CMAKE_CXX_STANDARD 17 CACHE STRING "c++ standard")
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    if (NOT CMAKE_CXX_STANDARD MATCHES "17|20|23|26")
        message(FATAL_ERROR "unsupported c++ standard version: ${CMAKE_CXX_STANDARD}")
    endif()

    option(ASSERTIONS_TESTING_ENABLED "enable testing through googletest" ON)
    option(ASSERTIONS_TREAT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." ON)

    sanitizers_init(ASSERTIONS)

    # TODO: g++ -Wall -Wextra -Wpedantic -Q --help=warning | grep disabled
    set(ASSERTIONS_WARNINGS_GCC
        -Wall
        -Wpedantic
        -Wextra
        -Wshadow
        $<$<BOOL:${ASSERTIONS_TREAT_WARNINGS_AS_ERRORS}>:-Werror>
    )

    set(ASSERTIONS_WARNINGS_MSVC
        /W4
        $<$<BOOL:${ASSERTIONS_TREAT_WARNINGS_AS_ERRORS}>:/WX>
    )

    set(ASSERTIONS_SOURCE_FILES
        assert.h
    )

    set(ASSERTIONS_INTERFACE_SOURCES "")
    foreach(FILE ${ASSERTIONS_SOURCE_FILES})
        list(
            APPEND ASSERTIONS_INTERFACE_SOURCES
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/assertions/${FILE}>"
                "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/assertions/${FILE}>"
        )
    endforeach()

    target_sources(assertions INTERFACE
        ${ASSERTIONS_INTERFACE_SOURCES}
    )

    if(MSVC)
        target_compile_options(assertions INTERFACE
            ${ASSERTIONS_WARNINGS_MSVC}
        )
    else()
        target_compile_options(assertions INTERFACE
            ${ASSERTIONS_WARNINGS_GCC}
        )
    endif()

    if(ASSERTIONS_TESTING_ENABLED)
        FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0)

        FetchContent_MakeAvailable(googletest)
    endif()

    if(ASSERTIONS_TESTING_ENABLED)
        add_executable(assertions_test
            test/assert.cpp
        )
        add_test(NAME assertions_test COMMAND assertions_test)
        target_link_libraries(assertions_test assertions::assertions gtest_main)
        target_compile_definitions(assertions_test PRIVATE ASSERTIONS_ENABLE)
        target_compile_options(assertions_test PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/FI windows.h /DNOMINMAX>)

        target_sanitize(ASSERTIONS assertions_test)
    endif()

    if(WIN32)
        group_sources(include)
        group_sources(test)
    endif()
endif()