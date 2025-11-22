cmake_minimum_required(VERSION 3.12)
project(DensePacking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)  # This ensures -std=c++17, not -std=gnu++17

# Get version from git
execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Provide a fallback if git is not available or no tags exist
if(NOT GIT_VERSION)
    set(GIT_VERSION "unknown")
endif()

# Find Kokkos
find_package(Kokkos REQUIRED)

# Find VTK
find_package(VTK REQUIRED)

# Note: prefer target-specific include directories (added after target creation)



set(DensePacking_FILES
    src/main.cxx
    src/Data.h
    src/Data.cxx
    src/YamlAPI.h
    src/YamlAPI.cxx
    src/SimulationConstants.h
    src/AModule.h
    src/AModule.cxx

    src/Reader.h
    src/Reader.cxx

        src/Writer.h
    src/Writer.cxx


        src/ContactSearch.h
    src/ContactSearch.cxx


        src/Forces.h
    src/Forces.cxx

    
        src/Time.h
    src/Time.cxx


            src/Timer.h
    src/Timer.cxx

                src/Integrator.h
    src/Integrator.cxx

    src/RadiusScaler.cxx
    src/RadiusScaler.h
)




add_executable(DensePacking ${DensePacking_FILES})
target_include_directories(DensePacking PRIVATE ${PROJECT_SOURCE_DIR}/src)

# Pass version to compiler
target_compile_definitions(DensePacking PRIVATE PROJECT_VERSION="${GIT_VERSION}")


# Find yaml-cpp
find_package(yaml-cpp REQUIRED)

# Link Kokkos, VTK, and yaml-cpp
target_link_libraries(DensePacking PRIVATE Kokkos::kokkos ${VTK_LIBRARIES} yaml-cpp)

# Set C++ standard
set_target_properties(DensePacking PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
