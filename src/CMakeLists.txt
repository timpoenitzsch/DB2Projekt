add_executable(main)
target_sources(main PRIVATE main.cpp)
target_link_libraries(main Catch2 cereal)
target_compile_features(main PRIVATE cxx_std_17)
set_property(TARGET main PROPERTY CXX_STANDARD 17)

enable_testing()
add_test(tests main)

add_subdirectory(core)
