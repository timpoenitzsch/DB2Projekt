
/*! \example dictionary_compressed_column.hpp
 * This is an example of how to implement a compression technique in our framework. One has to inherit from the abstract
 * base class CoGaDB::CompressedColumn and implement the pure virtual methods.
 */

#pragma once

#include "compressed_column.hpp"
#include "core/global_definitions.hpp"

namespace CoGaDB {

    /*!
     *  \brief     This class represents a dictionary compressed column with type T, is the base class for all
     * compressed typed column classes.
     */
    template<class T>
    class DictionaryCompressedColumn final : public CompressedColumn<DictionaryCompressedColumn, T> {
    public:
        using value_type = T;
        /***************** constructors and destructor *****************/
        explicit DictionaryCompressedColumn(std::string name) : CompressedColumn<DictionaryCompressedColumn, T>(std::move(name)) /*TODO: instantiate your attributes here*/ {}


        void insert_impl(const ColumnType &new_value) {
            /* TODO: implement */
        }

        void insert_impl(const value_type &new_value) {
            /* TODO: implement */
        }

        template<std::input_iterator InputIterator>
        void insert_impl(InputIterator first, InputIterator last) {
            /* TODO: implement */
        }

        void update_impl(TID tid, const ColumnType &new_value) {
            /* TODO: implement */
        }

        void update_impl(PositionList &tids, const ColumnType &new_value) {
            /* TODO: implement */
        }

        void remove_impl(TID tid) {
            /* TODO: implement */
        }

        // assumes tid list is sorted ascending
        void remove_impl(PositionList &tids) {
            /* TODO: implement */
        }

        void clearContent_impl() {
            /* TODO: implement */
        }

        ColumnType get_impl(TID tid) {
            /* TODO: implement */
        }

        [[nodiscard]] ColumnType get_impl(TID tid) const {
            /* TODO: implement */
        }

        [[nodiscard]] std::string print_impl() const noexcept {
            /* TODO: implement */
        }

        [[nodiscard]] size_t size_impl() const noexcept {
            /* TODO: implement */
        }

        [[nodiscard]] size_t getSizeInBytes_impl() const noexcept {
            /* TODO: implement */
        }

        [[nodiscard]] std::unique_ptr<DictionaryCompressedColumn> copy_impl() const {
            /* TODO: implement */
        }

        /**
         * @brief Serialization method called by Cereal. Implement this method in your compressed columns to get serialization working.
         */
        template<class Archive>
        void serialize(Archive &archive) {
            //archive();// serialize things by passing them to the archive
        }

    private:

        /* TODO: define your attributes and helper functions here */
    };

}// namespace CoGaDB
