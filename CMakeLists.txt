cmake_minimum_required(VERSION 3.16)

# Project settings
project(pairg VERSION 0.2.1 LANGUAGES CXX)

## Options
# Check if the user wants to only build the library
option(BUILD_LIBRARY_ONLY "Inform whether only library should be built" OFF)
# Check if the user wants to build google test applications
option(BUILD_TESTING "Inform whether test applications should be built" OFF)
# Check if the user wants to use bundled Kokkos library
option(USE_BUNDLED_KOKKOS "Use bundled Kokkos" OFF)
# Check if the user wants to use bundled KokkosKernels library
option(USE_BUNDLED_KOKKOS_KERNELS "Use bundled KokkosKernels" OFF)
# Check if the user wants to use all bundled dependencies
option(USE_BUNDLED_ALL "Use all bundled dependencies" OFF)

if(USE_BUNDLED_ALL)
  set(USE_BUNDLED_KOKKOS on)
  set(USE_BUNDLED_KOKKOS_KERNELS on)
endif(USE_BUNDLED_ALL)

# Include external modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
# Adding CMake module path.
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)
## Finding dependencies
find_package(OpenMP REQUIRED)
find_package(Kokkos QUIET)
find_package(KokkosKernels QUIET)
# Handle bundled dependencies
include(${PROJECT_SOURCE_DIR}/ext/external.cmake)

## Defining target library `libpairg`
# Defining the INTERFACE library
add_library(libpairg INTERFACE)
# Defining include directories
target_include_directories(libpairg
  INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>;$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)
# Defining link libraries
target_link_libraries(libpairg
  INTERFACE $<BUILD_INTERFACE:Kokkos::kokkos>;$<INSTALL_INTERFACE:Kokkos::kokkos>
  INTERFACE $<BUILD_INTERFACE:Kokkos::kokkoskernels>;$<INSTALL_INTERFACE:Kokkos::kokkoskernels>)
# Use C++14
target_compile_features(libpairg INTERFACE cxx_std_14)
# Setting header files
file(GLOB HEADER_FILES RELATIVE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/include/pairg/*.hpp")
list(TRANSFORM HEADER_FILES PREPEND "${PROJECT_SOURCE_DIR}/include/" OUTPUT_VARIABLE BUILD_HEADER_FILES)
list(TRANSFORM HEADER_FILES PREPEND "${CMAKE_INSTALL_FULL_INCLUDEDIR}/" OUTPUT_VARIABLE INSTALL_HEADER_FILES)
# Defining target sources
target_sources(libpairg
  INTERFACE "$<BUILD_INTERFACE:${BUILD_HEADER_FILES}>;$<INSTALL_INTERFACE:${INSTALL_HEADER_FILES}>")
# Defining `pairg::libpairg` alias
add_library(pairg::libpairg ALIAS libpairg)
# Install targets
install(FILES ${BUILD_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pairg)
# Exporting targets
install(TARGETS libpairg EXPORT pairg-targets)
install(EXPORT pairg-targets NAMESPACE pairg:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pairg)
# Creating the package configuration file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pairg-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/pairg-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pairg)
# Generating the version file for the configuration file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/pairg-config-version.cmake"
  VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
  COMPATIBILITY AnyNewerVersion)
# Install generated configuration files
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pairg-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/pairg-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pairg)

# Save libs and executables in the same place
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Output directory for libraries" )

# Build PairG command-line tool
if(NOT BUILD_LIBRARY_ONLY)
  add_subdirectory("${PROJECT_SOURCE_DIR}/src")
endif(NOT BUILD_LIBRARY_ONLY)

# Generate test executables if requested
if(BUILD_TESTING)
  add_subdirectory("${PROJECT_SOURCE_DIR}/tests")
endif(BUILD_TESTING)
