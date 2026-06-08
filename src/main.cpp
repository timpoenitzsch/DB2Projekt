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
#include <compression/run_length_compressed_column.hpp>
#include <compression/delta_compressed_column.hpp>

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
                                  (Column, DictionaryCompressedColumn, RunLengthCompressedColumn, DeltaCompressedColumn),
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

TEST_CASE("DictionaryCompressedColumn stores repeated values using dictionary semantics", "[dictionary]") {
    CoGaDB::DictionaryCompressedColumn<std::string> column("dictionary_test");

    column.insert(std::string("Magdeburg"));
    column.insert(std::string("Berlin"));
    column.insert(std::string("Magdeburg"));
    column.insert(std::string("Rostock"));
    column.insert(std::string("Berlin"));

    REQUIRE(column.size() == 5);

    REQUIRE(std::get<std::string>(column.get(0)) == "Magdeburg");
    REQUIRE(std::get<std::string>(column.get(1)) == "Berlin");
    REQUIRE(std::get<std::string>(column.get(2)) == "Magdeburg");
    REQUIRE(std::get<std::string>(column.get(3)) == "Rostock");
    REQUIRE(std::get<std::string>(column.get(4)) == "Berlin");

    column.update(2, std::string("Ilmenau"));

    REQUIRE(std::get<std::string>(column.get(0)) == "Magdeburg");
    REQUIRE(std::get<std::string>(column.get(1)) == "Berlin");
    REQUIRE(std::get<std::string>(column.get(2)) == "Ilmenau");
    REQUIRE(std::get<std::string>(column.get(3)) == "Rostock");
    REQUIRE(std::get<std::string>(column.get(4)) == "Berlin");

    column.remove(1);

    REQUIRE(column.size() == 4);
    REQUIRE(std::get<std::string>(column.get(0)) == "Magdeburg");
    REQUIRE(std::get<std::string>(column.get(1)) == "Ilmenau");
    REQUIRE(std::get<std::string>(column.get(2)) == "Rostock");
    REQUIRE(std::get<std::string>(column.get(3)) == "Berlin");
}


TEST_CASE("DictionaryCompressedColumn supports store and load", "[dictionary][serialization]") {
    CoGaDB::DictionaryCompressedColumn<int> source("dictionary_serialization_test");
    CoGaDB::DictionaryCompressedColumn<int> target("dictionary_serialization_test");

    source.insert(7);
    source.insert(7);
    source.insert(42);
    source.insert(-3);
    source.insert(42);

    REQUIRE_NOTHROW(source.store(DATA_PATH));

    target.load(DATA_PATH);

    REQUIRE(target.size() == source.size());

    for (CoGaDB::TID tid = 0; tid < source.size(); ++tid) {
        REQUIRE(std::get<int>(target.get(tid)) == std::get<int>(source.get(tid)));
    }
}


TEST_CASE("RunLengthCompressedColumn preserves long runs", "[rle]") {
    CoGaDB::RunLengthCompressedColumn<int> column("rle_test");

    column.insert(1);
    column.insert(1);
    column.insert(1);
    column.insert(2);
    column.insert(2);
    column.insert(3);
    column.insert(3);
    column.insert(3);
    column.insert(3);

    REQUIRE(column.size() == 9);

    std::vector<int> expected{1, 1, 1, 2, 2, 3, 3, 3, 3};

    for (CoGaDB::TID tid = 0; tid < expected.size(); ++tid) {
        REQUIRE(std::get<int>(column.get(tid)) == expected[tid]);
    }
}


TEST_CASE("RunLengthCompressedColumn rebuilds runs correctly after update and remove", "[rle]") {
    CoGaDB::RunLengthCompressedColumn<std::string> column("rle_update_remove_test");

    column.insert(std::string("A"));
    column.insert(std::string("A"));
    column.insert(std::string("B"));
    column.insert(std::string("B"));
    column.insert(std::string("C"));

    column.update(2, std::string("A"));

    std::vector<std::string> after_update{"A", "A", "A", "B", "C"};

    REQUIRE(column.size() == after_update.size());

    for (CoGaDB::TID tid = 0; tid < after_update.size(); ++tid) {
        REQUIRE(std::get<std::string>(column.get(tid)) == after_update[tid]);
    }

    column.remove(3);

    std::vector<std::string> after_remove{"A", "A", "A", "C"};

    REQUIRE(column.size() == after_remove.size());

    for (CoGaDB::TID tid = 0; tid < after_remove.size(); ++tid) {
        REQUIRE(std::get<std::string>(column.get(tid)) == after_remove[tid]);
    }
}


TEST_CASE("RunLengthCompressedColumn supports store and load", "[rle][serialization]") {
    CoGaDB::RunLengthCompressedColumn<int> source("rle_serialization_test");
    CoGaDB::RunLengthCompressedColumn<int> target("rle_serialization_test");

    std::vector<int> values{5, 5, 5, 10, 10, -1, -1, -1, -1};

    for (const auto value: values) {
        source.insert(value);
    }

    REQUIRE_NOTHROW(source.store(DATA_PATH));

    target.load(DATA_PATH);

    REQUIRE(target.size() == values.size());

    for (CoGaDB::TID tid = 0; tid < values.size(); ++tid) {
        REQUIRE(std::get<int>(target.get(tid)) == values[tid]);
    }
}


TEST_CASE("DeltaCompressedColumn stores arithmetic deltas correctly", "[delta]") {
    CoGaDB::DeltaCompressedColumn<int> column("delta_int_test");

    std::vector<int> values{100, 103, 106, 105, 120, 90};

    for (const auto value: values) {
        column.insert(value);
    }

    REQUIRE(column.size() == values.size());

    for (CoGaDB::TID tid = 0; tid < values.size(); ++tid) {
        REQUIRE(std::get<int>(column.get(tid)) == values[tid]);
    }
}


TEST_CASE("DeltaCompressedColumn supports float values", "[delta]") {
    CoGaDB::DeltaCompressedColumn<float> column("delta_float_test");

    std::vector<float> values{1.5F, 2.0F, 2.25F, -1.0F, 10.75F};

    for (const auto value: values) {
        column.insert(value);
    }

    REQUIRE(column.size() == values.size());

    for (CoGaDB::TID tid = 0; tid < values.size(); ++tid) {
        REQUIRE_THAT(std::get<float>(column.get(tid)), Catch::Matchers::WithinULP(values[tid], 10));
    }
}


TEST_CASE("DeltaCompressedColumn handles string front-coding-style values", "[delta][string]") {
    CoGaDB::DeltaCompressedColumn<std::string> column("delta_string_test");

    std::vector<std::string> values{
        "Ein String",
        "Ein String2",
        "Eine Zeichenkette",
        "Eine Zweite Zeichenkette",
        "Noch ein String"
    };

    for (const auto &value: values) {
        column.insert(value);
    }

    REQUIRE(column.size() == values.size());

    for (CoGaDB::TID tid = 0; tid < values.size(); ++tid) {
        REQUIRE(std::get<std::string>(column.get(tid)) == values[tid]);
    }
}


TEST_CASE("DeltaCompressedColumn rebuilds representation after update and remove", "[delta]") {
    CoGaDB::DeltaCompressedColumn<int> column("delta_update_remove_test");

    column.insert(10);
    column.insert(13);
    column.insert(16);
    column.insert(20);

    column.update(1, 12);

    std::vector<int> after_update{10, 12, 16, 20};

    REQUIRE(column.size() == after_update.size());

    for (CoGaDB::TID tid = 0; tid < after_update.size(); ++tid) {
        REQUIRE(std::get<int>(column.get(tid)) == after_update[tid]);
    }

    column.remove(2);

    std::vector<int> after_remove{10, 12, 20};

    REQUIRE(column.size() == after_remove.size());

    for (CoGaDB::TID tid = 0; tid < after_remove.size(); ++tid) {
        REQUIRE(std::get<int>(column.get(tid)) == after_remove[tid]);
    }
}


TEST_CASE("DeltaCompressedColumn supports store and load", "[delta][serialization]") {
    CoGaDB::DeltaCompressedColumn<int> source("delta_serialization_test");
    CoGaDB::DeltaCompressedColumn<int> target("delta_serialization_test");

    std::vector<int> values{1000, 1001, 1005, 990, 1200};

    for (const auto value: values) {
        source.insert(value);
    }

    REQUIRE_NOTHROW(source.store(DATA_PATH));

    target.load(DATA_PATH);

    REQUIRE(target.size() == values.size());

    for (CoGaDB::TID tid = 0; tid < values.size(); ++tid) {
        REQUIRE(std::get<int>(target.get(tid)) == values[tid]);
    }
}