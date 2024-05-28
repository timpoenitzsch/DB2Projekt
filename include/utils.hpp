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

#include "compression/compressed_column.hpp"
#include "core/column.hpp"
#include "core/column_base.hpp"
#include "core/global_definitions.hpp"
#include <catch2/internal/catch_floating_point_helpers.hpp>
#include <catch2/internal/catch_polyfills.hpp>
#include <random>
#include <string>

using namespace CoGaDB;

namespace {

// borrowed this from catch anonymous namespace
        template <typename FP>
        bool almostEqualUlps(FP lhs, FP rhs, uint64_t maxUlpDiff) {
            // Comparison with NaN should always be false.
            // This way we can rule it out before getting into the ugly details
            if (Catch::isnan(lhs) || Catch::isnan(rhs)) {
                return false;
            }

            // This should also handle positive and negative zeros, infinities
            const auto ulpDist = Catch::ulpDistance(lhs, rhs);

            return ulpDist <= maxUlpDiff;
        }

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