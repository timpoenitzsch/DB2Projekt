add_executable(main main.cpp)
target_link_libraries(main Catch2::Catch2WithMain cereal -fsanitize=address -fsanitize=undefined -fsanitize=pointer-compare -fsanitize=leak)
target_compile_options(main PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -fPIE -fPIC -Wall -Wextra -Wpedantic -Werror -fsanitize=address -fsanitize=undefined -fsanitize=pointer-compare -fsanitize=leak>
        )

check_pie_supported(OUTPUT_VARIABLE PIE)
target_compile_features(main PRIVATE cxx_std_20)
set_target_properties(main PROPERTIES CXX_STANDARD 20
                                POSITION_INDEPENDENT_CODE PIE
                                CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")

add_test(main main)