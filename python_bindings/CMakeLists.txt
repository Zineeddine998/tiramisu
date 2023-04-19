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

set(NUMPY_VER 1.20.3)
find_package(NumPy ${NUMPY_VER} QUIET)
if (NOT NumPy_FOUND)
    message(STATUS "NumPy not found. Downloading from repository...")
    include(FetchContent)
    FetchContent_Declare(numpy_repo
                         GIT_REPOSITORY https://github.com/numpy/numpy.git
                         GIT_TAG v${NUMPY_VER})
    FetchContent_GetProperties(numpy_repo)
    if (NOT numpy_repo_POPULATED)
        FetchContent_Populate(numpy_repo)
        add_subdirectory(${numpy_repo_SOURCE_DIR} ${numpy_repo_BINARY_DIR})
    endif()
    find_package(NumPy ${NUMPY_VER} REQUIRED)
endif ()

##
# Add our sources to this sub-tree.
##

add_subdirectory(src)