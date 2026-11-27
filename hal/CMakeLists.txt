# CMakeList.txt for HAL
#   Build a library (`hal`) which exposes the header files as "hal/*.h"
#   Use header as: #include "hal/button.h"

include_directories(hal/include)
file(GLOB MY_SOURCES "src/*.c")

add_library(hal STATIC ${MY_SOURCES})

target_include_directories(hal PUBLIC include)

# libgpiod (copied from the board into lib/gpiod)
target_include_directories(hal PUBLIC ${CMAKE_SOURCE_DIR}/lib/gpiod)
target_link_directories(hal PUBLIC ${CMAKE_SOURCE_DIR}/lib/gpiod)
target_link_libraries(hal PUBLIC gpiod)
