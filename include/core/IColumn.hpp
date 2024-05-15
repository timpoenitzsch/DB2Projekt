#pragma once
#include <type_traits>
#include <concepts>
#include "global_definitions.hpp"

namespace CoGaDB {

    template<class C, class T>
    concept IColumn = requires(C c) {
        { c.insert_impl(ColumnType()) };
        { c.insert_impl(T()) };
        { c.update_impl(TID(), ColumnType()) };
        { c.insert_impl(std::input_iterator<T>, std::input_iterator<T>) };
        { c.update_impl(PositionList(), ColumnType()) };
        { c.remove_impl(TID()) };
        { c.remove_impl(PositionList()) };
        { c.clearContent_impl() };
        { c.clearContent_impl() };
        { c.get_impl(TID()) } -> std::same_as<ColumnType>;
        { c.print_impl() } -> std::same_as<std::string>;
        { c.size_impl() } -> std::same_as<std::size_t>;
        { c.getSizeInBytes_impl() } -> std::same_as<std::size_t>;
        { c.isMaterialized_impl() } -> std::convertible_to<bool>;
        { c.isCompressed_impl() } -> std::convertible_to<bool>;
        { c.getName_impl() } -> std::same_as<std::string>;
        { c.getType() } -> std::same_as<AttributeType>;
        { c==c } -> std::convertible_to<bool>;

        // operator<<?
    };

}