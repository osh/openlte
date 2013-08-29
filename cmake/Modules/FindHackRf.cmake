INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_HACKRF libhackrf)

FIND_PATH(
    HACKRF_INCLUDE_DIRS
    NAMES libhackrf/hackrf.h
    HINTS $ENV{HACKRF_DIR}/include
        ${PC_HACKRF_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    HACKRF_LIBRARIES
    NAMES hackrf
    HINTS $ENV{HACKRF_DIR}/lib
        ${PC_HACKRF_LIBDIR}
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HACKRF DEFAULT_MSG HACKRF_LIBRARIES HACKRF_INCLUDE_DIRS)
MARK_AS_ADVANCED(HACKRF_LIBRARIES HACKRF_INCLUDE_DIRS)
