
#include "config.hpp"                           // for DATA_PATH
#include "core/column.hpp"                      // for Column
#include "core/global_definitions.hpp"          // for CoGaDB, TID
#include "tests/utils.hpp"                      // for isEqual, gen, getAt...
#include <catch2/catch_template_test_macros.hpp>// for TEMPLATE_PRODUCT_TE...
#include <catch2/catch_test_macros.hpp>         // for operator""_catch_sr
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/matchers/catch_matchers.hpp>// for REQUIRE_THAT
#include <memory>                            // for unique_ptr
#include <random>                            // for uniform_int_distrib...
#include <string>                            // for string
#include <vector>                            // for vector
/*
 * TODO: include your class headers here
 * #include "compression/dictionary_compressed_column.hpp"
 *
 */

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
                                  (Column /*TODO: put your classes here, e.g. DictionaryCompressedColumn*/),
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

    reference_data = GENERATE_REF(take(1, chunk(TEST_DATA_SIZE, GeneratorWrapper<ValueType>(Catch::Detail::make_unique<RandomColumnDataGenerator<InputType, ValueType>>(low, high)))));
    for (const auto &val: reference_data) {
        REQUIRE_NOTHROW(col_one.insert(val));
    }

    REQUIRE(reference_data.size() == col_one.size());
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));


    /****** VIRTUAL COPY CONSTRUCTOR TEST ******/
    auto copy = col_one.copy();
    REQUIRE(copy);
    REQUIRE(col_one == *copy);

    /****** UPDATE TEST ******/
    TID tid = GENERATE(take(1, random(0, TEST_DATA_SIZE)));
    auto new_value = GENERATE_REF(take(1, GeneratorWrapper<ValueType>(Catch::Detail::make_unique<RandomColumnDataGenerator<InputType, ValueType>>(low, high))));

    reference_data[tid] = new_value;

    REQUIRE_NOTHROW(col_one.update(tid, new_value));
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));

    tid = GENERATE_REF(take(1, random(static_cast<TID>(0), reference_data.size())));
    new_value = GENERATE_REF(take(1, GeneratorWrapper<ValueType>(Catch::Detail::make_unique<RandomColumnDataGenerator<InputType, ValueType>>(low, high))));

    reference_data[tid] = new_value;

    REQUIRE_NOTHROW(col_one.update(tid, new_value));
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));

    /****** DELETE TEST ******/
    tid = GENERATE(take(1, random(0, TEST_DATA_SIZE)));

    reference_data.erase(reference_data.begin() + tid);

    REQUIRE_NOTHROW(col_one.remove(tid));
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));

    /****** STORE AND LOAD TEST ******/
    REQUIRE_NOTHROW(col_one.store(DATA_PATH));
    col_one.clearContent();
    REQUIRE(col_one.size() == 0);

    REQUIRE_NOTHROW(col_two.load(DATA_PATH));
    REQUIRE_THAT(col_two, isEqual<TestType>(reference_data));
}
