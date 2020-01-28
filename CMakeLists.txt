project(adl2edl C CXX)

cmake_minimum_required(VERSION 2.8)

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

set(CMAKE_BUILD_TYPE DEBUG)

## Brave enough?
#add_definitions(-Wall)
add_definitions(-fstack-protector-all)

include_directories(
 ${X11_INCLUDE_DIR}
 ${MOTIF_INCLUDE_DIR}
)

add_executable(adl2edl
 fonts.cc translator.cc graphic.cc control.cc monitor.cc
 fonts.h translator.h
)
target_link_libraries(adl2edl
 ${MOTIF_LIBRARIES}
 ${X11_LIBRARIES}
 ${X11_Xt_LIB}
)

add_executable(adlMakeColorList
 makeColorList.cc
)

install(TARGETS adl2edl adlMakeColorList
 RUNTIME DESTINATION bin
)
