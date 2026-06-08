#pragma once

#include "compressed_column.hpp"
#include "core/global_definitions.hpp"

#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <algorithm>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace CoGaDB {

    /*!
     * \brief Run Length Encoded column.
     *
     * Consecutive equal values are represented as one value and its run length.
     * Example: A A A B B C -> values_: A B C, counts_: 3 2 1
     */
    template<class T>
    class RunLengthCompressedColumn : public CompressedColumn<RunLengthCompressedColumn, T> {
    public:
        using value_type = T;

        explicit RunLengthCompressedColumn(std::string name)
            : CompressedColumn<RunLengthCompressedColumn, T>(std::move(name)) {}

        void insert_impl(const ColumnType &new_value) {
            insert_impl(std::get<T>(new_value));
        }

        void insert_impl(const value_type &new_value) {
            if (!values_.empty() && values_.back() == new_value) {
                ++counts_.back();
            } else {
                values_.push_back(new_value);
                counts_.push_back(1);
            }

            ++size_;
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
                throw std::out_of_range("RunLengthCompressedColumn::update_impl: tid out of range");
            }

            materialized[tid] = std::get<T>(new_value);
            rebuildFromMaterialized(materialized);
        }

        void update_impl(const PositionList &tids, const ColumnType &new_value) {
            auto materialized = materialize();
            const auto value = std::get<T>(new_value);

            for (const auto tid: tids) {
                if (tid >= materialized.size()) {
                    throw std::out_of_range("RunLengthCompressedColumn::update_impl: tid out of range");
                }

                materialized[tid] = value;
            }

            rebuildFromMaterialized(materialized);
        }

        void remove_impl(TID tid) {
            auto materialized = materialize();

            if (tid >= materialized.size()) {
                throw std::out_of_range("RunLengthCompressedColumn::remove_impl: tid out of range");
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
                    throw std::out_of_range("RunLengthCompressedColumn::remove_impl: tid out of range");
                }

                materialized.erase(std::next(materialized.begin(), static_cast<std::ptrdiff_t>(tid)));
            }

            rebuildFromMaterialized(materialized);
        }

        void clearContent_impl() {
            values_.clear();
            counts_.clear();
            size_ = 0;
        }

        ColumnType get_impl(TID tid) {
            return static_cast<const RunLengthCompressedColumn &>(*this).get_impl(tid);
        }

        [[nodiscard]] ColumnType get_impl(TID tid) const {
            if (tid >= size_) {
                throw std::out_of_range("RunLengthCompressedColumn::get_impl: tid out of range");
            }

            std::size_t current_position = 0;

            for (std::size_t run = 0; run < counts_.size(); ++run) {
                const auto next_position = current_position + counts_[run];

                if (tid < next_position) {
                    return values_[run];
                }

                current_position = next_position;
            }

            throw std::out_of_range("RunLengthCompressedColumn::get_impl: invalid run structure");
        }

        [[nodiscard]] std::string print_impl() const noexcept {
            std::ostringstream out;

            out << "[";

            bool first_value = true;

            for (std::size_t run = 0; run < values_.size(); ++run) {
                for (std::size_t i = 0; i < counts_[run]; ++i) {
                    if (!first_value) {
                        out << ", ";
                    }

                    out << values_[run];
                    first_value = false;
                }
            }

            out << "]";

            return out.str();
        }

        [[nodiscard]] size_t size_impl() const noexcept {
            return size_;
        }

        [[nodiscard]] size_t getSizeInBytes_impl() const noexcept {
            std::size_t size = 0;

            size += values_.capacity() * sizeof(T);
            size += counts_.capacity() * sizeof(std::size_t);
            size += sizeof(size_);

            return size;
        }

        [[nodiscard]] std::unique_ptr<RunLengthCompressedColumn> copy_impl() const {
            return std::make_unique<RunLengthCompressedColumn>(*this);
        }

        template<class Archive>
        void serialize(Archive &archive) {
            archive(values_, counts_, size_);
        }

    private:
        std::vector<T> values_;
        std::vector<std::size_t> counts_;
        std::size_t size_ = 0;

        [[nodiscard]] std::vector<T> materialize() const {
            std::vector<T> result;
            result.reserve(size_);

            for (std::size_t run = 0; run < values_.size(); ++run) {
                result.insert(result.end(), counts_[run], values_[run]);
            }

            return result;
        }

        void rebuildFromMaterialized(const std::vector<T> &materialized) {
            clearContent_impl();

            for (const auto &value: materialized) {
                insert_impl(value);
            }
        }
    };

}// namespace CoGaDB