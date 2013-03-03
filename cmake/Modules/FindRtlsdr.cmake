INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_RTLSDR librtlsdr)

FIND_PATH(
    RTLSDR_INCLUDE_DIRS
    NAMES rtl-sdr.h
    HINTS $ENV{RTLSDR_DIR}/include
        ${PC_RTLSDR_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    RTLSDR_LIBRARIES
    NAMES rtlsdr
    HINTS $ENV{RTLSDR_DIR}/lib
        ${PC_RTLSDR_LIBDIR}
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RTLSDR DEFAULT_MSG RTLSDR_LIBRARIES RTLSDR_INCLUDE_DIRS)
MARK_AS_ADVANCED(RTLSDR_LIBRARIES RTLSDR_INCLUDE_DIRS)
