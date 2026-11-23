# sat_test: prints live satellite azimuth/elevation (TLE mode) or receives
# targets from GPredict (rotctld mode). No HAL dependency, so it also runs on
# the host PC for testing.

add_executable(sat_test src/main.c)
target_link_libraries(sat_test PRIVATE sat)

# Copy to the board only when cross-compiling (keeps host builds working)
if(CMAKE_CROSSCOMPILING)
  add_custom_command(TARGET sat_test POST_BUILD
    COMMAND "scp" "$<TARGET_FILE:sat_test>" "${CMAKE_SOURCE_DIR}/tle/sample.txt" "${CMAKE_SOURCE_DIR}/tle/fetch_tle.sh" "beagley@192.168.7.2:~"
    COMMENT "Copying sat_test + sample TLE to beagley HOME directory")
endif()
