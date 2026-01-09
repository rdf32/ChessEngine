cmake_minimum_required(VERSION 3.20)
project(chess_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Required for Windows DLL exports
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Find Python
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Manually add pybind11 include path
include_directories("C:/Users/rylie/miniconda3/envs/chess/Lib/site-packages/pybind11/include")

# Build the Python extension
add_library(chess_engine MODULE
    bindings.cpp
    chess.cpp
    logger.cpp
)

# Remove the default "lib" prefix on Windows, and ensure .pyd suffix
set_target_properties(chess_engine PROPERTIES
    PREFIX ""
    SUFFIX ".pyd"
)

# Link against Python libraries
target_link_libraries(chess_engine PRIVATE Python::Python)
