if(NOT TARGET Kokkos::kokkos)
  if(NOT USE_BUNDLED_KOKKOS)
    message(FATAL_ERROR "Kokkos library not found. "
      "Pass in `-DUSE_BUNDLED_KOKKOS=on` when running cmake to use the bundled version. "
      "It will be installed alongside the library.")
  endif()
  message(STATUS "Using bundled Kokkos library")
  set(Kokkos_SOURCE_DIR ${PROJECT_SOURCE_DIR}/ext/kokkos)
  execute_process(
    COMMAND git submodule update --init --recursive -- ${Kokkos_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(${Kokkos_SOURCE_DIR})
endif()

if(NOT TARGET Kokkos::kokkoskernels)
  if(NOT USE_BUNDLED_KOKKOS_KERNELS)
    message(FATAL_ERROR "KokkosKernels library not found. "
      "Pass in `-DUSE_BUNDLED_KOKKOS_KERNELS=on` when running cmake to use the bundled "
      "version. It will be installed alongside the library.")
  endif()
  message(STATUS "Using bundled kokkos-kernels library")
  set(KokkosKernels_SOURCE_DIR ${PROJECT_SOURCE_DIR}/ext/kokkos-kernels)
  execute_process(
    COMMAND git submodule update --init --recursive -- ${KokkosKernels_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(${KokkosKernels_SOURCE_DIR})
endif()
