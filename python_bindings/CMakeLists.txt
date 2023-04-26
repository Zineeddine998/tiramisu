##
# Load Python dependencies, including external pybind11
##

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

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
set(NUMPY_VERSION "1.24.2")
find_package(NumPy ${NUMPY_VERSION} REQUIRED)


include_directories(${NUMPY_INCLUDE_DIRS})

link_directories(${Numpy_LIBRARY_DIR})

##
# Add our sources to this sub-tree.
##

add_subdirectory(src)