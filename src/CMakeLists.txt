add_executable(main main.cpp)
target_link_libraries(main Catch2::Catch2WithMain cereal -fsanitize=address -fsanitize=undefined -fsanitize=pointer-compare -fsanitize=leak)
target_compile_options(main PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -Wall -Wextra -Wpedantic -Werror -fsanitize=address -fsanitize=undefined -fsanitize=pointer-compare -fsanitize=leak>
        )
target_compile_features(main PRIVATE cxx_std_20)
set_property(TARGET main PROPERTY CXX_STANDARD 20)

add_test(main main)
