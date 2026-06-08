#include <config.hpp>                                       // for DATA_PATH
#include <core/column.hpp>                                  // for Column
#include <core/global_definitions.hpp>                      // for TID
#include <utils.hpp>                                        // for almostE...
#include <catch2/catch_get_random_seed.hpp>                 // for getSeed
#include <catch2/catch_template_test_macros.hpp>            // for TEMPLAT...
#include <catch2/catch_test_macros.hpp>                     // for operato...
#include <catch2/generators/catch_generators.hpp>           // for GENERAT...
#include <catch2/generators/catch_generators_adapters.hpp>  // for take
#include <catch2/generators/catch_generators_random.hpp>    // for random
#include <catch2/matchers/catch_matchers.hpp>               // for REQUIRE...
#include <catch2/matchers/catch_matchers_floating_point.hpp>// for WithinULP
#include <catch2/matchers/catch_matchers_range_equals.hpp>  // for RangeEq...
#include <iterator>                                         // for next
#include <random>                                           // for mt19937
#include <string>                                           // for string
#include <type_traits>                                      // for conditi...
#include <vector>                                           // for vector
#include <compression/dictionary_compressed_column.hpp>

constexpr int TEST_DATA_SIZE = 100;

template<typename C>
struct Column_Test_Fixture {
    using ValueType = typename C::value_type;
    Column_Test_Fixture() : col_one(getAttributeString<ValueType>()),
                            col_two(getAttributeString<ValueType>()) {}

    C col_one;
    C col_two;
};

using namespace CoGaDB;


TEMPLATE_PRODUCT_TEST_CASE_METHOD(Column_Test_Fixture,
                                  "Template test case method with test types specified inside std::tuple",
                                  "[class][template]",
                                  (Column, DictionaryCompressedColumn),
                                  (int, float, std::string)) {
    using ValueType = typename Column_Test_Fixture<TestType>::ValueType;
    using InputType = typename std::conditional_t<std::is_same_v<ValueType, std::string>, int, ValueType>;
    auto &col_one = Column_Test_Fixture<TestType>::col_one;
    auto &col_two = Column_Test_Fixture<TestType>::col_two;
    std::mt19937 gen(Catch::getSeed());
    constexpr int low = std::is_same_v<ValueType, std::string> ? 0 : -100;
    constexpr int high = 100;


    /****** INSERT TEST ******/
    std::vector<ValueType> reference_data;
    REQUIRE(col_one.size() == 0);
    REQUIRE(col_two.size() == 0);

    reference_data = GENERATE_REF(take(1, chunk(TEST_DATA_SIZE, GeneratorWrapper<ValueType>(Catch::Detail::make_unique<RandomColumnDataGenerator<InputType, ValueType>>(low, high)))));
    for (const auto &val: reference_data) {
        REQUIRE_NOTHROW(col_one.insert(val));
    }

    REQUIRE(reference_data.size() == col_one.size());

    if constexpr (std::is_same_v<ValueType, float>)
        REQUIRE_THAT(col_one, Catch::Matchers::RangeEquals(reference_data, [](auto lhs, auto rhs) -> auto { return almostEqualUlps(lhs, rhs, 10); }));
    else
        REQUIRE_THAT(col_one, Catch::Matchers::RangeEquals(reference_data));


    /****** VIRTUAL COPY CONSTRUCTOR TEST ******/
    auto copy = col_one.copy();
    REQUIRE(copy);
    REQUIRE_THAT(col_one, Catch::Matchers::RangeEquals(*copy));

    /****** ==-Operator TEST ******/
    REQUIRE(col_one == *copy);

    /****** UPDATE TESTS ******/
    TID tid = GENERATE(take(1, random(0, TEST_DATA_SIZE-1)));
    auto new_value = GENERATE_REF(take(1, GeneratorWrapper<ValueType>(Catch::Detail::make_unique<RandomColumnDataGenerator<InputType, ValueType>>(low, high))));

    reference_data[tid] = new_value;

    REQUIRE_NOTHROW(col_one.update(tid, new_value));
    if constexpr (std::is_same_v<ValueType, float>) {
        REQUIRE_THAT(col_one[tid], Catch::Matchers::WithinULP(col_one[tid], 10));
        REQUIRE_THAT(col_one, Catch::Matchers::RangeEquals(reference_data, [](auto lhs, auto rhs) -> auto { return almostEqualUlps(lhs, rhs, 10); }));
    } else {
        REQUIRE(col_one[tid] == reference_data[tid]);
        REQUIRE_THAT(col_one, Catch::Matchers::RangeEquals(reference_data));
    }

    tid = GENERATE_REF(take(1, random(TID{}, reference_data.size()-1)));
    new_value = GENERATE_REF(take(1, GeneratorWrapper<ValueType>(Catch::Detail::make_unique<RandomColumnDataGenerator<InputType, ValueType>>(low, high))));

    reference_data[tid] = new_value;

    REQUIRE_NOTHROW(col_one.update(tid, new_value));
    if constexpr (std::is_same_v<ValueType, float>) {
        REQUIRE_THAT(col_one[tid], Catch::Matchers::WithinULP(col_one[tid], 10));
        REQUIRE_THAT(col_one, Catch::Matchers::RangeEquals(reference_data, [](auto lhs, auto rhs) -> auto { return almostEqualUlps(lhs, rhs, 10); }));
    } else {
        REQUIRE(col_one[tid] == reference_data[tid]);
        REQUIRE_THAT(col_one, Catch::Matchers::RangeEquals(reference_data));
    }

    /****** DELETE TEST ******/
    tid = GENERATE(take(1, random(0, TEST_DATA_SIZE-1)));

    reference_data.erase(std::next(reference_data.begin(), tid));

    REQUIRE_NOTHROW(col_one.remove(tid));
    REQUIRE(col_one.size() == reference_data.size());

    if constexpr (std::is_same_v<ValueType, float>)
        REQUIRE_THAT(col_one, Catch::Matchers::RangeEquals(reference_data, [](auto lhs, auto rhs) -> auto { return almostEqualUlps(lhs, rhs, 10); }));
    else
        REQUIRE_THAT(col_one, Catch::Matchers::RangeEquals(reference_data));

    /****** STORE AND LOAD TEST ******/
    REQUIRE_NOTHROW(col_one.store(DATA_PATH));
    col_one.clearContent();
    REQUIRE(col_one.size() == 0);

    REQUIRE_NOTHROW(col_two.load(DATA_PATH));
    REQUIRE(col_two.size() == reference_data.size());
    if constexpr (std::is_same_v<ValueType, float>)
        REQUIRE_THAT(col_two, Catch::Matchers::RangeEquals(reference_data, [](auto lhs, auto rhs) -> auto { return almostEqualUlps(lhs, rhs, 10); }));
    else
        REQUIRE_THAT(col_two, Catch::Matchers::RangeEquals(reference_data));
}
