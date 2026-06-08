/*! \example dictionary_compressed_column.hpp
 * This is an example of how to implement a compression technique in our framework. One has to inherit from the abstract
 * base class CoGaDB::CompressedColumn and implement the pure virtual methods.
 */

// Dictionary compression stores each distinct value once and keeps only
// dictionary codes in the logical column representation.
// Updates and removals do not compact the dictionary to keep codes stable.

#pragma once

#include "compressed_column.hpp"
#include "core/global_definitions.hpp"

#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <algorithm>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace CoGaDB {

    /*!
     *  \brief This class represents a dictionary compressed column with type T.
     */
    template<class T>
    class DictionaryCompressedColumn : public CompressedColumn<DictionaryCompressedColumn, T> {
    public:
        using value_type = T;

        /***************** constructors and destructor *****************/
        explicit DictionaryCompressedColumn(std::string name)
            : CompressedColumn<DictionaryCompressedColumn, T>(std::move(name)) {}

        void insert_impl(const ColumnType &new_value) {
            insert_impl(std::get<T>(new_value));
        }

        void insert_impl(const value_type &new_value) {
            const auto code = getOrCreateCode(new_value);
            codes_.push_back(code);
        }

        template<std::input_iterator InputIterator, std::sentinel_for<InputIterator> Sentinel>
        void insert_impl(const InputIterator &first, const Sentinel &last) {
            for (auto it = first; it != last; ++it) {
                insert_impl(*it);
            }
        }

        void update_impl(TID tid, const ColumnType &new_value) {
            if (tid >= codes_.size()) {
                throw std::out_of_range("DictionaryCompressedColumn::update_impl: tid out of range");
            }

            const auto code = getOrCreateCode(std::get<T>(new_value));
            codes_[tid] = code;
        }

        void update_impl(const PositionList &tids, const ColumnType &new_value) {
            const auto code = getOrCreateCode(std::get<T>(new_value));

            for (const auto tid: tids) {
                if (tid >= codes_.size()) {
                    throw std::out_of_range("DictionaryCompressedColumn::update_impl: tid out of range");
                }

                codes_[tid] = code;
            }
        }

        void remove_impl(TID tid) {
            if (tid >= codes_.size()) {
                throw std::out_of_range("DictionaryCompressedColumn::remove_impl: tid out of range");
            }

            codes_.erase(std::next(codes_.begin(), static_cast<std::ptrdiff_t>(tid)));
        }

        // assumes tid list is sorted ascending
        void remove_impl(const PositionList &tids) {
            for (auto it = tids.rbegin(); it != tids.rend(); ++it) {
                remove_impl(*it);
            }
        }

        void clearContent_impl() {
            dictionary_.clear();
            value_to_code_.clear();
            codes_.clear();
        }

        ColumnType get_impl(TID tid) {
            return static_cast<const DictionaryCompressedColumn &>(*this).get_impl(tid);
        }

        [[nodiscard]] ColumnType get_impl(TID tid) const {
            if (tid >= codes_.size()) {
                throw std::out_of_range("DictionaryCompressedColumn::get_impl: tid out of range");
            }

            const auto code = codes_[tid];

            if (code >= dictionary_.size()) {
                throw std::out_of_range("DictionaryCompressedColumn::get_impl: invalid dictionary code");
            }

            return dictionary_[code];
        }

        [[nodiscard]] std::string print_impl() const noexcept {
            std::ostringstream out;

            out << "[";

            for (std::size_t i = 0; i < codes_.size(); ++i) {
                if (i > 0) {
                    out << ", ";
                }

                const auto code = codes_[i];
                out << dictionary_[code];
            }

            out << "]";

            return out.str();
        }

        [[nodiscard]] size_t size_impl() const noexcept {
            return codes_.size();
        }

        [[nodiscard]] size_t getSizeInBytes_impl() const noexcept {
            std::size_t size = 0;

            size += dictionary_.capacity() * sizeof(T);
            size += codes_.capacity() * sizeof(std::size_t);
            size += value_to_code_.size() * (sizeof(T) + sizeof(std::size_t));

            return size;
        }

        [[nodiscard]] std::unique_ptr<DictionaryCompressedColumn> copy_impl() const {
            return std::make_unique<DictionaryCompressedColumn>(*this);
        }

        /**
         * @brief Serialization method called by Cereal. Implement this method in your compressed columns to get serialization working.
         */
        template<class Archive>
        void serialize(Archive &archive) {
            archive(dictionary_, value_to_code_, codes_);
        }

    private:
        std::vector<T> dictionary_;
        std::map<T, std::size_t> value_to_code_;
        std::vector<std::size_t> codes_;

        std::size_t getOrCreateCode(const T &value) {
            const auto it = value_to_code_.find(value);

            if (it != value_to_code_.end()) {
                return it->second;
            }

            const auto code = dictionary_.size();
            dictionary_.push_back(value);
            value_to_code_.emplace(value, code);

            return code;
        }
    };

}// namespace CoGaDB