# Find 'HTSlib' library.
#
# This set the following variables:
#   - HTSlib_FOUND
#   - HTSlib_VERSION
#   - HTSlib_INCLUDEDIR
#   - HTSlib_LIBRARIES
#
# and the following imported targets:
#   - HTSlib::HTSlib

if(HTSlib_INCLUDEDIR)
  set(HTSlib_FIND_QUIETLY TRUE)
else()
  # Try pkg-config, first.
  find_package(PkgConfig QUIET)
  pkg_check_modules(HTSlib QUIET htslib)
  # If HTSlib_INCLUDEDIR is not set, this searches for the header file.
  find_path(HTSlib_INCLUDEDIR htslib/hts.h)
  find_library(HTSlib_LIBRARIES hts)
endif(HTSlib_INCLUDEDIR)

## handle the QUIETLY and REQUIRED arguments and set HTSlib_FOUND to TRUE if
## all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HTSlib DEFAULT_MSG HTSlib_INCLUDEDIR HTSlib_LIBRARIES)

mark_as_advanced(HTSlib_FOUND HTSlib_VERSION HTSlib_INCLUDEDIR HTSlib_LIBRARIES)

# Define `HTSlib::HTSlib` imported target
if(HTSlib_FOUND AND NOT TARGET HTSlib::HTSlib)
  add_library(HTSlib::HTSlib INTERFACE IMPORTED)
  set_target_properties(HTSlib::HTSlib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${HTSlib_INCLUDEDIR}"
    INTERFACE_LINK_LIBRARIES "${HTSlib_LIBRARIES}"
    INTERFACE_LINK_DIRECTORIES "${HTSlib_LIBDIR}")
endif()
