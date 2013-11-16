INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_UHD uhd)

FIND_PATH(
    UHD_INCLUDE_DIRS
    NAMES uhd/version.hpp
    HINTS $ENV{UHD_DIR}/include
        ${PC_UHD_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    UHD_LIBRARIES
    NAMES uhd
    HINTS $ENV{UHD_DIR}/lib
        ${PC_UHD_LIBDIR}
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(UHD DEFAULT_MSG UHD_LIBRARIES UHD_INCLUDE_DIRS)
MARK_AS_ADVANCED(UHD_LIBRARIES UHD_INCLUDE_DIRS)
