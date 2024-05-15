/*
 * File:   unittest.hpp
 * Author: ameister
 *
 * Created on 3. Juni 2016, 15:59
 */

#pragma once
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <compression/compressed_column.hpp>
#include <core/column.hpp>
#include <core/column_base.hpp>
#include <core/global_definitions.hpp>
#include <random>
#include <string>

using namespace CoGaDB;

namespace {

template<typename T, IColumnType R = T>
class RandomColumnDataGenerator : public Catch::Generators::IGenerator<R> {
    mutable std::mt19937 gen;
    const T begin, end;
    mutable R cur_val;

    R get_rand_value() const {
        return gen();
    }
    bool next() override {
        return true;
    }

public:
    RandomColumnDataGenerator(T low, T high) : gen(Catch::getSeed()), begin{low}, end{high} {}

    R const&get() const override;
};



template<>
float RandomColumnDataGenerator<float,float>::get_rand_value() const {
    auto dist = std::uniform_real_distribution(begin, end);
    return dist(gen);
}

template<>
int RandomColumnDataGenerator<int,int>::get_rand_value() const {
    auto dist = std::uniform_int_distribution(begin, end);
    return dist(gen);
}

template<>
std::string RandomColumnDataGenerator<int, std::string>::get_rand_value() const {
    std::string s;
    auto characters = std::uniform_int_distribution('a', 'z');
    auto num_char = std::uniform_int_distribution(begin, end);

    std::generate_n(std::back_inserter(s), num_char(gen), [&characters, this]() { return characters(gen); });

    return s;
}
template<typename T, IColumnType R>
R const &RandomColumnDataGenerator<T, R>::get() const {
    cur_val = RandomColumnDataGenerator<T, R>::get_rand_value();
    return cur_val;
}
} // end anonymous namespaces

template<class Column>
class ColumnComparator : public Catch::Matchers::MatcherBase<Column> {
    std::vector<typename Column::value_type> ref_data;

public:
    explicit ColumnComparator(decltype(ref_data) &reference_data) : ref_data(reference_data) {}

    // Performs the test for this matcher
    bool match(Column const &col) const override {
        if (ref_data.size() != col.size()) {
            std::cerr << "Size mismatch!" << std::endl;
            return false;
        }
        for (unsigned int i = 0; i < ref_data.size(); i++) {
            typename Column::value_type col_value = const_cast<Column &>(col)[i];
            if (ref_data[i] != col_value) {
                std::cerr << "Fatal Error! In Unittest: read invalid data" << std::endl;
                std::cerr << "Column: '" << col.getName() << "' TID: '" << i << "' Expected Value: '" << ref_data[i]
                          << "' Actual Value: '" << col_value << "'" << std::endl;
                return false;
            }
        }
        return true;
    }

    // Produces a string describing what this matcher does. It should
    // include any provided data (the begin/ end in this case) and
    // be written as if it were stating a fact (in the output it will be
    // preceded by the value under test).
    [[nodiscard]] std::string describe() const override {
        std::ostringstream ss;
        ss << "column equals reference data";
        return ss.str();
    }
};

// The builder function
template<class Column>
inline ColumnComparator<Column> isEqual(std::vector<typename Column::value_type> &ref_data) {
    return ColumnComparator<Column>(ref_data);
}

template<typename ValueType>
AttributeType getAttributeType() {
    return AttributeType::INT;
}

template<>
AttributeType getAttributeType<int>() {
    return AttributeType::INT;
}
template<>
AttributeType getAttributeType<float>() {
    return AttributeType::FLOAT;
}

template<>
AttributeType getAttributeType<std::string>() {
    return AttributeType::VARCHAR;
}

template<typename ValueType>
std::string getAttributeString() {
    return "unknown column";
}

template<>
std::string getAttributeString<int>() {
    return "int column";
}
template<>
std::string getAttributeString<float>() {
    return "float column";
}

template<>
std::string getAttributeString<std::string>() {
    return "string column";
}