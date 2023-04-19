##
# Load Python dependencies, including external pybind11
##

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

set(PYBIND11_VER 2.10.2)
find_package(pybind11 ${PYBIND11_VER} QUIET)
if (NOT pybind11_FOUND)
    include(FetchContent)
    FetchContent_Declare(pybind11
                         GIT_REPOSITORY https://github.com/pybind/pybind11.git
                         GIT_TAG v${PYBIND11_VER})
    FetchContent_MakeAvailable(pybind11)
endif ()



message(STATUS "Getting numpy 1.24.2")
include(ExternalProject)

set(NUMPY_VERSION "1.24.2")
set(NUMPY_URL "https://github.com/numpy/numpy/archive/v${NUMPY_VERSION}.tar.gz")

ExternalProject_Add(numpy
    PREFIX numpy
    URL ${NUMPY_URL}
    URL_HASH SHA256=<insert hash of tarball here>
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

set(NUMPY_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/numpy/src/numpy/numpy/core/include)
set(NUMPY_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/numpy/src/numpy/build/lib.linux-x86_64-${CMAKE_BUILD_TYPE}/numpy/core/libnumpy.a)

add_executable(myexecutable main.cpp)
target_include_directories(myexecutable PRIVATE ${NUMPY_INCLUDE_DIR})
target_link_libraries(myexecutable PRIVATE ${NUMPY_LIBRARIES})

##
# Add our sources to this sub-tree.
##

add_subdirectory(src)