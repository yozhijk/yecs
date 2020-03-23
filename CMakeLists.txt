cmake_minimum_required(VERSION 3.12)

project(yecs)

# Global settings
if (WIN32)
    add_definitions(/MP -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
elseif (UNIX)
    add_definitions(-Wno-ignored-attributes)
endif (WIN32)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin)
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_DEBUG_POSTFIX "-d")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Allow IDE's to group targets into folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(YECS_ENABLE_TESTING "Enable unit tests" ON)
add_subdirectory(yecs)

if (YECS_ENABLE_TESTING)
    add_subdirectory(tests)
    set(gtest_force_shared_crt ON CACHE BOOL "Use /MD and /MDd" FORCE)
    add_subdirectory(third_party/gtest)
    set(THIRD_PARTY_TARGETS
        gtest
        gtest_main
        gmock
        gmock_main)

    foreach(TGT ${THIRD_PARTY_TARGETS})
        set_property(TARGET ${TGT} PROPERTY FOLDER "third_party")
    endforeach()
endif()

set(TF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/cpp-taskflow)


