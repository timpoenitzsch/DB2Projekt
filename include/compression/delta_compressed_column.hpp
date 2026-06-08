#pragma once

#include "compressed_column.hpp"
#include "core/global_definitions.hpp"

#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// Delta coding stores the first value explicitly and all following numeric
// values as differences to their predecessor. Strings are handled by a
// front-coding-like representation using common prefixes.

namespace CoGaDB {

    /*!
     * \brief Delta compressed column.
     *
     * For arithmetic types, the first value is stored explicitly and all following
     * values are stored as deltas to their predecessor.
     *
     * For std::string, a front-coding-like representation is used:
     * the first string is stored explicitly, following strings are represented by
     * common-prefix length and remaining suffix.
     */
    template<class T>
    class DeltaCompressedColumn : public CompressedColumn<DeltaCompressedColumn, T> {
    public:
        using value_type = T;

        explicit DeltaCompressedColumn(std::string name)
            : CompressedColumn<DeltaCompressedColumn, T>(std::move(name)) {}

        void insert_impl(const ColumnType &new_value) {
            insert_impl(std::get<T>(new_value));
        }

        void insert_impl(const value_type &new_value) {
            if constexpr (std::is_arithmetic_v<T>) {
                insertArithmetic(new_value);
            } else {
                insertString(new_value);
            }
        }

        template<std::input_iterator InputIterator, std::sentinel_for<InputIterator> Sentinel>
        void insert_impl(const InputIterator &first, const Sentinel &last) {
            for (auto it = first; it != last; ++it) {
                insert_impl(*it);
            }
        }

        void update_impl(TID tid, const ColumnType &new_value) {
            auto materialized = materialize();

            if (tid >= materialized.size()) {
                throw std::out_of_range("DeltaCompressedColumn::update_impl: tid out of range");
            }

            materialized[tid] = std::get<T>(new_value);
            rebuildFromMaterialized(materialized);
        }

        void update_impl(const PositionList &tids, const ColumnType &new_value) {
            auto materialized = materialize();
            const auto value = std::get<T>(new_value);

            for (const auto tid: tids) {
                if (tid >= materialized.size()) {
                    throw std::out_of_range("DeltaCompressedColumn::update_impl: tid out of range");
                }

                materialized[tid] = value;
            }

            rebuildFromMaterialized(materialized);
        }

        void remove_impl(TID tid) {
            auto materialized = materialize();

            if (tid >= materialized.size()) {
                throw std::out_of_range("DeltaCompressedColumn::remove_impl: tid out of range");
            }

            materialized.erase(std::next(materialized.begin(), static_cast<std::ptrdiff_t>(tid)));
            rebuildFromMaterialized(materialized);
        }

        // assumes tid list is sorted ascending
        void remove_impl(const PositionList &tids) {
            auto materialized = materialize();

            for (auto it = tids.rbegin(); it != tids.rend(); ++it) {
                const auto tid = *it;

                if (tid >= materialized.size()) {
                    throw std::out_of_range("DeltaCompressedColumn::remove_impl: tid out of range");
                }

                materialized.erase(std::next(materialized.begin(), static_cast<std::ptrdiff_t>(tid)));
            }

            rebuildFromMaterialized(materialized);
        }

        void clearContent_impl() {
            first_value_ = T{};
            arithmetic_deltas_.clear();

            first_string_.clear();
            prefix_lengths_.clear();
            suffixes_.clear();

            size_ = 0;
        }

        ColumnType get_impl(TID tid) {
            return static_cast<const DeltaCompressedColumn &>(*this).get_impl(tid);
        }

        [[nodiscard]] ColumnType get_impl(TID tid) const {
            if (tid >= size_) {
                throw std::out_of_range("DeltaCompressedColumn::get_impl: tid out of range");
            }

            if constexpr (std::is_arithmetic_v<T>) {
                return getArithmetic(tid);
            } else {
                return getString(tid);
            }
        }

        [[nodiscard]] std::string print_impl() const noexcept {
            std::ostringstream out;
            const auto values = materialize();

            out << "[";

            for (std::size_t i = 0; i < values.size(); ++i) {
                if (i > 0) {
                    out << ", ";
                }

                out << values[i];
            }

            out << "]";

            return out.str();
        }

        [[nodiscard]] size_t size_impl() const noexcept {
            return size_;
        }

        [[nodiscard]] size_t getSizeInBytes_impl() const noexcept {
            std::size_t result = sizeof(size_);

            if constexpr (std::is_arithmetic_v<T>) {
                result += sizeof(first_value_);
                result += arithmetic_deltas_.capacity() * sizeof(T);
            } else {
                result += sizeof(first_string_) + first_string_.capacity();
                result += prefix_lengths_.capacity() * sizeof(std::size_t);
                result += suffixes_.capacity() * sizeof(std::string);

                for (const auto &suffix: suffixes_) {
                    result += suffix.capacity();
                }
            }

            return result;
        }

        [[nodiscard]] std::unique_ptr<DeltaCompressedColumn> copy_impl() const {
            return std::make_unique<DeltaCompressedColumn>(*this);
        }

        template<class Archive>
        void serialize(Archive &archive) {
            archive(first_value_,
                    arithmetic_deltas_,
                    first_string_,
                    prefix_lengths_,
                    suffixes_,
                    size_);
        }

    private:
        T first_value_{};
        std::vector<T> arithmetic_deltas_;

        std::string first_string_;
        std::vector<std::size_t> prefix_lengths_;
        std::vector<std::string> suffixes_;

        std::size_t size_ = 0;

        void insertArithmetic(const T &new_value) {
            if (size_ == 0) {
                first_value_ = new_value;
            } else {
                const auto previous_value = getArithmeticValue(size_ - 1);
                arithmetic_deltas_.push_back(new_value - previous_value);
            }

            ++size_;
        }

        T getArithmeticValue(TID tid) const {
            T value = first_value_;

            for (std::size_t i = 0; i < tid; ++i) {
                value += arithmetic_deltas_[i];
            }

            return value;
        }

        ColumnType getArithmetic(TID tid) const {
            return getArithmeticValue(tid);
        }

        void insertString(const std::string &new_value) {
            if (size_ == 0) {
                first_string_ = new_value;
            } else {
                const auto previous_value = getStringValue(size_ - 1);
                const auto prefix_length = commonPrefixLength(previous_value, new_value);

                prefix_lengths_.push_back(prefix_length);
                suffixes_.push_back(new_value.substr(prefix_length));
            }

            ++size_;
        }

        std::string getStringValue(TID tid) const {
            if (tid == 0) {
                return first_string_;
            }

            std::string value = first_string_;

            for (std::size_t i = 0; i < tid; ++i) {
                value = value.substr(0, prefix_lengths_[i]) + suffixes_[i];
            }

            return value;
        }

        ColumnType getString(TID tid) const {
            return getStringValue(tid);
        }

        [[nodiscard]] std::vector<T> materialize() const {
            std::vector<T> result;
            result.reserve(size_);

            for (std::size_t tid = 0; tid < size_; ++tid) {
                if constexpr (std::is_arithmetic_v<T>) {
                    result.push_back(getArithmeticValue(tid));
                } else {
                    result.push_back(getStringValue(tid));
                }
            }

            return result;
        }

        void rebuildFromMaterialized(const std::vector<T> &materialized) {
            clearContent_impl();

            for (const auto &value: materialized) {
                insert_impl(value);
            }
        }

        static std::size_t commonPrefixLength(const std::string &lhs, const std::string &rhs) {
            const auto max_length = std::min(lhs.size(), rhs.size());
            std::size_t prefix_length = 0;

            while (prefix_length < max_length && lhs[prefix_length] == rhs[prefix_length]) {
                ++prefix_length;
            }

            return prefix_length;
        }
    };

}// namespace CoGaDB