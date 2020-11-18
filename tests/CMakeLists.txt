add_executable(c64_tests
        tests-main.cpp
        test_6502_timings.cpp
        test_c64.cpp
        test_cpu_6502.cpp
        test_functional_tests.cpp
        test_addressing_modes.cpp
        common.hpp
        common.cpp
        tools.cpp
        tools.hpp
)

target_link_libraries(c64_tests c64)

set(ROM_DIR ${CMAKE_CURRENT_BINARY_DIR}/roms)

configure_file(klaus_roms/6502_functional_test.bin ${ROM_DIR}/6502_functional_test.bin COPYONLY)
configure_file(timingtest/timingtest-1.bin ${ROM_DIR}/timingtest.bin COPYONLY)

# copy of the C64 roms required
configure_file(../roms/basic.901226-01.bin ${ROM_DIR}/basic.bin COPYONLY)
configure_file(../roms/characters.901225-01.bin ${ROM_DIR}/char.bin COPYONLY)
configure_file(../roms/kernal.901227-03.bin ${ROM_DIR}/kernal.bin COPYONLY)