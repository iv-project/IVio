cmake_minimum_required (VERSION 3.14)

# check if this is the main project
if (NOT DEFINED PROJECT_NAME)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /EHsc")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wpedantic -fPIC")
    endif()
    set(ROOT_PROJECT TRUE)
else()
    set(ROOT_PROJECT FALSE)
endif ()

project(IVio)

option(PYTHON_BINDINGS "compile python bindings" OFF)

add_subdirectory(src/ivio)

if (${PYTHON_BINDINGS})
    add_subdirectory(lib/pybind11)
    add_subdirectory(src/iviopy)
else()
    if (${ROOT_PROJECT})
        find_package (fmt REQUIRED)

        add_subdirectory(lib/Catch2)

        enable_testing()
        add_subdirectory(docs/snippets)
        add_subdirectory(src/test_ivio)
    endif()
endif()
