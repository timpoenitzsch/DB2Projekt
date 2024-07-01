#pragma once

#include <core/column_base.hpp>

namespace CoGaDB
{

    /*!
     *
     *
     *  \brief This class represents a compressed column with type T, is the base class for all compressed typed
     * column classes and allows a uniform handling of compressed columns of a certain type T. \details   This class is
     * intended to be a base class, so it has a virtual destructor and pure virtual methods, which need to be
     * implemented in a derived class. \author    Sebastian Breß \version   0.2 \date      2013 \copyright GNU LESSER
     * GENERAL PUBLIC LICENSE - Version 3, http://www.gnu.org/licenses/lgpl-3.0.txt
     */
    template<template <typename> class Impl, class T>
    class CompressedColumn : public ColumnBase<Impl, T>
    {
      public:
        /***************** constructors and destructor *****************/
          explicit CompressedColumn(std::string name) : ColumnBase<Impl,T>(std::move(name)) {}


        [[nodiscard]] bool isMaterialized_impl() const noexcept
        {
            return false;
        }

        [[nodiscard]] bool isCompressed_impl() const noexcept
        {
            return true;
        }
    };
} // namespace CoGaDB
