include(GrPlatform)
add_library(tools
  src/libtools_socket_wrap.cc
)
include_directories(hdr ${CMAKE_SOURCE_DIR}/cmn_hdr)
