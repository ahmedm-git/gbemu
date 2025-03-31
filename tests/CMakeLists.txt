# tests/CMakeLists.txt
add_executable(my_tests test_cpu.cpp)
target_compile_options(my_tests PRIVATE
  -Wall -Wextra -pedantic -g
)
target_link_libraries(my_tests PRIVATE
  Catch2::Catch2WithMain
  GameboyLib
)

# Register tests with CTest
include(Catch)
catch_discover_tests(my_tests)
