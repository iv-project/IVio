cmake_minimum_required (VERSION 3.15)

project (io-playground VERSION 1.0.0)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE
         Release
         CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE
    )
endif ()

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list (APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/seqan3/build_system")
list (APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/b.i.o./build_system")
list (APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/seqan/util/cmake")

include (seqan_require_ccache)
seqan_require_ccache ()

find_package (ZLIB QUIET REQUIRED)
find_package (BZip2 QUIET REQUIRED)

set (SEQAN3_SUBMODULES_DIR "${CMAKE_CURRENT_LIST_DIR}/lib")
find_package (SeqAn3 QUIET REQUIRED)

set (BIO_DEPENDENCY_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/lib/seqan3/include")
find_package (BIO QUIET REQUIRED)

set (SEQAN_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/seqan/include")
find_package (SeqAn QUIET REQUIRED)
add_library (seqan_seqan INTERFACE)
target_compile_definitions (seqan_seqan INTERFACE ${SEQAN_DEFINITIONS})
target_compile_options (seqan_seqan INTERFACE ${SEQAN_CXX_FLAGS})
target_link_libraries (seqan_seqan INTERFACE "${SEQAN_LIBRARIES}")
target_include_directories (seqan_seqan INTERFACE "${SEQAN_INCLUDE_DIRS}")
add_library (seqan::seqan ALIAS seqan_seqan)

add_library (io_interface INTERFACE)
target_compile_options (io_interface INTERFACE "-pedantic" "-Wall" "-Wextra" "-Wno-interference-size")
target_include_directories (io_interface INTERFACE "${CMAKE_CURRENT_LIST_DIR}/lib/seqan3/test/include")

add_subdirectory (enrico)
add_subdirectory (svenja)

### Add where needed ###
# include (seqan_require_test)
# seqan_require_test ()

# include (seqan_require_benchmark)
# seqan_require_benchmark ()

### Usage ###
# add_executable ("program" program.cpp)
# target_link_libraries ("program" PUBLIC seqan3::seqan3 bio::bio gtest_main benchmark_main)
