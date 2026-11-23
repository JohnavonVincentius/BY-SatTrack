# CMakeLists.txt for the satellite library (`sat`)
#   SGP4 propagator, TLE parser, az/el look angles, rotctld (GPredict) server.
#   Pure C + libm + sockets: builds on the host PC and on the BeagleY-AI.
#   Use headers as: #include "sat/sgp4.h"

file(GLOB SAT_SOURCES "src/*.c")
add_library(sat STATIC ${SAT_SOURCES})
target_include_directories(sat PUBLIC include)
target_link_libraries(sat PUBLIC m)
