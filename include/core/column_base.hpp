#pragma once

#include "IColumn.hpp"
#include "cereal/archives/portable_binary.hpp"
#include <algorithm>
#include <any>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>


/*! \brief The global namespace of the programming tasks, to avoid name clashes with other libraries.*/
namespace CoGaDB {

    /*!
     *
     *
     *  \brief     This class represents a column with type T, is the base class for all typed column classes and allows
     * a uniform handling of columns of a certain type T. \details   This class is intended to be a base class, so it
     * has a virtual destructor and pure virtual methods, which need to be implemented in a derived class. Furthermore,
     * it declares pure virtual methods to allow a generic handling of typed columns, e.g., operator[]. All algorithms
     * can be applied to a typed column, because of this operator. This abstracts from a columns implementation detail,
     * e.g., whether they are compressed or not. \author    Sebastian Breß \version   0.2 \date      2013 \copyright GNU
     * LESSER GENERAL PUBLIC LICENSE - Version 3, http://www.gnu.org/licenses/lgpl-3.0.txt
     */

    template<template <typename> class D, IColumnType T>
    class ColumnBase {

        using Derived = D<T>;
        std::string name_;
        constexpr Derived &self() { return *static_cast<Derived *>(this); }
        constexpr const Derived &self() const { return *static_cast<const Derived * const >(this); }

    public:
        using value_type = T;
        const AttributeType attribute_type = ValueAttributeTypeMapper<value_type>().attribute_type;
        /***************** constructors and destructor *****************/
        //inherit constructor
        explicit ColumnBase(std::string name) requires IColumn<Derived, value_type>: name_(std::move(name)) { }

        /*! \brief returns database type of column (as defined in "SQL" statement)*/
        [[nodiscard]] AttributeType getType() const {
            return attribute_type;
        }

        [[nodiscard]] std::string getName() const noexcept {
            return name_;
        }

        [[nodiscard]] bool isMaterialized() const noexcept { return self().isMaterialized_impl(); }

        /*! \brief use this method to determine whether the column is materialized or a Lookup Column
         * \return true in case the column is storing the compressed values and false otherwise.*/
        [[nodiscard]] bool isCompressed() const noexcept { return self().isCompressed_impl(); };

                /***************** methods *****************/
        void insert(const ColumnType &new_Value) { self().insert_impl(new_Value); }
        void insert(const value_type &new_Value) { self().insert_impl(new_Value); }


        template<std::input_iterator InputIterator, std::sentinel_for<InputIterator> Sentinel>
        void insert(InputIterator first, Sentinel last) {
            self().insert_impl(first, last);
        }

        /*! \brief updates the value on position tid with a value new_Value, throws if an error occurs */
        void update(const TID tid, const ColumnType &new_Value) { self().update_impl(tid, new_Value); }

        /*! \brief updates the values specified by the position list with a value new_Value , throws if an error occurs*/
        void update(const PositionList &tids, const ColumnType &new_value) { self().update_impl(tids, new_value); }

        /*! \brief deletes the value on position tid, throws if an error occurs*/
        void remove(const TID tid) { self().remove_impl(tid); }

        /*! \brief deletes the values defined in the position list
         *  \details assumes tid list is sorted ascending, throws if an error occurs*/
        void remove(const PositionList &tids) { self().remove_impl(tids); }

        /*! \brief deletes all values stored in the column throws am exception if an error occurs */
        void clearContent() { self().clearContent_impl(); }

        /*! \brief generic function for fetching a value form a column (slow)
         *  \details check whether the object is valid (e.g., when a tid is not valid, then the returned object is
         * invalid as well) \return object of type ColumnType containing the value on position tid. If tid is not valid, throws exception. */
        [[nodiscard]] ColumnType get(const TID tid) { return self().get_impl(tid); } // not const, because operator [] does not provide const return type
        [[nodiscard]] ColumnType get(const TID tid) const { return self().get_impl(tid); } // not const, because operator [] does not provide const return type
        // and the child classes rely on []
        /*! \brief creates a textual representation of the content of the column */
        [[nodiscard]] std::string print() noexcept { return self().print_impl(); }

        /*! \brief returns the number of values (rows) in a column*/
        [[nodiscard]] size_t size() const noexcept { return self().size_impl(); }

        /*! \brief returns the size in bytes the column consumes in main memory*/
        [[nodiscard]] [[maybe_unused]] size_t getSizeInBytes() const noexcept { return self().getSizeInBytes_impl(); }

        /*! \brief virtual copy constructor
         * \return a ColumnPtr to an exact copy of the current column*/
        [[nodiscard]] std::unique_ptr<Derived> copy() { return self().copy_impl(); }

        /***************** relational operations on Columns which return lookup tables *****************/
        [[maybe_unused]] PositionList sort(SortOrder order);

        [[maybe_unused]] PositionList selection(const ColumnType &value_for_comparison, ValueComparator comp);

        [[maybe_unused]] PositionList parallel_selection(const ColumnType &,
                                        ValueComparator,
                                        unsigned int);

        // join algorithms
        template<IColumn<T> C>
        [[maybe_unused]]  PositionListPair hash_join(C &join_column);

        template<IColumn<T> C>
        [[maybe_unused]]  PositionListPair sort_merge_join(C &join_column);

        template<IColumn<T> C>
        [[maybe_unused]]  PositionListPair nested_loop_join(C join_column);

        bool add(const ColumnType &new_value);
        // vector addition between columns
        template<IColumn<T> C>
        bool add(C &column);

        [[maybe_unused]]  bool minus(const ColumnType &new_value);
        template<IColumn<T> C>
        [[maybe_unused]]  bool minus(C &join_column);

        [[maybe_unused]]  bool multiply(const ColumnType &new_value);

        template<IColumn<T> C>
        [[maybe_unused]] bool multiply(C &join_column);

        [[maybe_unused]] bool division(const ColumnType &new_value);

        template<IColumn<T> C>
        [[maybe_unused]] bool division(C &column);

        void store(const std::string &path_);

        void load(const std::string &path_);

        /*! \brief defines operator[] for this class, which enables the user to thread all typed columns as arrays.
         * \details Note that this method is pure virtual, so it has to be defined in a derived class.
         * \return a reference to the value at position index
         * */
        value_type operator[](TID index) {
            return std::get<value_type>(self().get(index));
        };

        bool operator==(const Derived &column) const;

        bool operator!=(const Derived &column) const {
            return !(self()==column);
        }

        friend std::ostream &operator<<(std::ostream &output, const Derived &base) {
            output << base.print_impl();
            return output;
        }
    };



}// namespace CoGaDB

#include "impl/column_base.tpp"
