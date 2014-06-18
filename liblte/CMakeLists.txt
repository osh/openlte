include(GrPlatform)
add_library(lte
  src/liblte_interface.cc
  src/liblte_phy.cc
  src/liblte_mac.cc
  src/liblte_rlc.cc
  src/liblte_rrc.cc
  src/liblte_mme.cc
)
include_directories(hdr ${CMAKE_SOURCE_DIR}/cmn_hdr)
