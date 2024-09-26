include(ExternalProject)

# Set default cmake build type to RelWithDebInfo during prototyping
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo"
    FORCE)
endif(NOT CMAKE_BUILD_TYPE)

## This block bundles VGio
#find_package(Protobuf REQUIRED)
#
## Find "HTSlib"
#find_package(HTSlib QUIET)
## Use bundled "HTSlib" if it is not installed
#if(NOT TARGET HTSlib::HTSlib)
#  set(HTSlib_SOURCE_DIR ${PROJECT_SOURCE_DIR}/ext/htslib)
#  ExternalProject_Add(htslib_git
#    DOWNLOAD_COMMAND git -C ${PROJECT_SOURCE_DIR} submodule update --init --recursive -- ${HTSlib_SOURCE_DIR}
#    SOURCE_DIR ${HTSlib_SOURCE_DIR}
#    BUILD_IN_SOURCE 1
#    CONFIGURE_COMMAND ""
#    BUILD_COMMAND make
#    INSTALL_COMMAND make install PREFIX=<INSTALL_DIR>)
#  ExternalProject_Get_Property(htslib_git INSTALL_DIR)
#  set(CMAKE_PREFIX_PATH "$<BUILD_INTERFACE:${INSTALL_DIR}/lib/pkgconfig>;$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/lib/pkgconfig>" ${CMAKE_PREFIX_PATH})
#  find_package(HTSlib REQUIRED)
#endif()
#
## Find "VGio"
#find_package(VGio QUIET)
## Use bundled "VGio" if it is not installed
#if(NOT TARGET VGio::VGio)
#  set(VGio_SOURCE_DIR ${PROJECT_SOURCE_DIR}/ext/libvgio)
#  execute_process(COMMAND git submodule update --init --recursive -- ${VGio_SOURCE_DIR}
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#  add_subdirectory(${VGio_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/ext/htslib)
#endif()
#
## If external libraries are added, the target depends on them assuring build
## order in parallel builds.
#if(TARGET htslib_git)
#  add_dependencies(pairg htslib_git)
#endif()

find_package(VGio REQUIRED)

###generate main executable
add_executable(pairg "${PROJECT_SOURCE_DIR}/src/main.cpp")
target_include_directories(pairg
  PRIVATE "${PROJECT_SOURCE_DIR}/ext"
  PRIVATE VGio::VGio)
target_link_libraries(pairg
  PRIVATE pairg::libpairg
  PRIVATE VGio::VGio)
