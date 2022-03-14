add_executable(main)
target_sources(main PRIVATE main.cpp base_column.cpp)
target_link_libraries(main Catch2 cereal)
target_compile_features(main PRIVATE cxx_std_17)

enable_testing()
add_test(main main)
