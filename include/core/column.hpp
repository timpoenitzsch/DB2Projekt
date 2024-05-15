#pragma once

#include "IColumn.hpp"
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <core/column_base.hpp>
#include <fstream>
#include <iostream>
#include <numeric>
#include <ranges>

namespace CoGaDB {

    template<IColumnType T>
    class Column : public ColumnBase<Column, T> {
    public:
        using value_type = T;
        /***************** constructors and destructor *****************/
        explicit Column(std::string name);

        void insert_impl(const ColumnType &new_value);
        void insert_impl(const value_type &new_value);
        template<std::input_iterator InputIterator>
        void insert_impl(InputIterator first, InputIterator last);

        void update_impl(TID tid, const ColumnType &new_value);
        void update_impl(PositionList &tids, const ColumnType &new_value);

        void remove_impl(TID tid);
        // assumes tid list is sorted ascending
        void remove_impl(PositionList &tids);

        void clearContent_impl();

        ColumnType get_impl(TID tid);
        [[nodiscard]] ColumnType get_impl(TID tid) const;

        [[nodiscard]] std::string print_impl() const noexcept;

        [[nodiscard]] size_t size_impl() const noexcept;

        [[nodiscard]] size_t getSizeInBytes_impl() const noexcept;

        [[nodiscard]] std::unique_ptr<Column<T>> copy_impl() const;

        [[nodiscard]] bool isMaterialized_impl() const noexcept {
            return true;
        }

        [[nodiscard]] bool isCompressed_impl() const noexcept {
            return false;
        }

        /**
         * @brief Serialization method called by Cereal. Implement this method in your compressed columns to get serialization working.
         */
        template<class Archive>
        void serialize(Archive &archive);

    private:
        struct Type_TID_Comparator {
            inline bool operator()(std::pair<T, TID> i, std::pair<T, TID> j) {
                return (i.first < j.first);
            }
        } type_tid_comparator;

        /*! values*/
        std::vector<T> values_;
    };

}// namespace CoGaDB

/* Here you can find the hidden implementation */
#include "impl/column.tpp"
