
/*! \example dictionary_compressed_column.hpp
 * This is an example of how to implement a compression technique in our framework. One has to inherit from an abstract
 * base class CoGaDB::CompressedColumn and implement the pure virtual methods.
 */

#pragma once

#include "compressed_column.hpp"
#include "core/global_definitions.hpp"

namespace CoGaDB
{

    /*!
     *  \brief     This class represents a dictionary compressed column with type T, is the base class for all
     * compressed typed column classes.
     */
    template<class T>
    class DictionaryCompressedColumn : public CompressedColumn<T>
    {
      public:
        /***************** constructors and destructor *****************/
        DictionaryCompressedColumn(const std::string &name, AttributeType db_type);
        virtual ~DictionaryCompressedColumn();

        virtual bool insert(const ColumnType &new_Value);
        virtual bool insert(const T &new_value);
        template<typename InputIterator>
        bool insert(InputIterator first, InputIterator last);

        virtual bool update(TID tid, const ColumnType &new_value);
        virtual bool update(PositionList &tid, const ColumnType &new_value);

        virtual bool remove(TID tid);
        // assumes tid list is sorted ascending
        virtual bool remove(PositionList &tid);
        virtual bool clearContent();

        virtual ColumnType get(TID tid);
        // virtual const std::any* const getRawData()=0;
        virtual void print() const noexcept;
        [[nodiscard]] virtual size_t size() const noexcept;
        [[nodiscard]] virtual unsigned int getSizeinBytes() const noexcept;

        [[nodiscard]] virtual std::unique_ptr<ColumnBase> copy() const;

        virtual void store(const std::string &path);
        virtual void load(const std::string &path);

        virtual T &operator[](int index);
        virtual T &operator[](int index) const;
    };

    /***************** Start of Implementation Section ******************/

    template<class T>
    DictionaryCompressedColumn<T>::DictionaryCompressedColumn(const std::string &name, AttributeType db_type) :
        CompressedColumn<T>(name, db_type)
    {
    }

    template<class T>
    DictionaryCompressedColumn<T>::~DictionaryCompressedColumn() = default;

    template<class T>
    bool DictionaryCompressedColumn<T>::insert(const ColumnType &)
    {
        return false;
    }

    template<class T>
    bool DictionaryCompressedColumn<T>::insert(const T &)
    {
        return false;
    }

    template<typename T>
    template<typename InputIterator>
    bool DictionaryCompressedColumn<T>::insert(InputIterator, InputIterator)
    {
        return true;
    }

    template<class T>
    ColumnType DictionaryCompressedColumn<T>::get(TID)
    {
        return {};
    }

    template<class T>
    void DictionaryCompressedColumn<T>::print() const noexcept
    {
    }
    template<class T>
    size_t DictionaryCompressedColumn<T>::size() const noexcept
    {
        return 0;
    }
    template<class T>
    std::unique_ptr<ColumnBase> DictionaryCompressedColumn<T>::copy() const
    {
        return {};
    }

    template<class T>
    bool DictionaryCompressedColumn<T>::update(TID, const ColumnType &)
    {
        return false;
    }

    template<class T>
    bool DictionaryCompressedColumn<T>::update(PositionList &, const ColumnType &)
    {
        return false;
    }

    template<class T>
    bool DictionaryCompressedColumn<T>::remove(TID)
    {
        return false;
    }

    template<class T>
    bool DictionaryCompressedColumn<T>::remove(PositionList &)
    {
        return false;
    }

    template<class T>
    bool DictionaryCompressedColumn<T>::clearContent()
    {
        return false;
    }

    template<class T>
    void DictionaryCompressedColumn<T>::store(const std::string &)
    {
    }
    template<class T>
    void DictionaryCompressedColumn<T>::load(const std::string &)
    {
    }

    template<class T>
    T &DictionaryCompressedColumn<T>::operator[](const int)
    {
        static T t;
        return t;
    }

    template<class T>
    T &DictionaryCompressedColumn<T>::operator[](const int) const
    {
        static T t;
        return t;
    }

    template<class T>
    unsigned int DictionaryCompressedColumn<T>::getSizeinBytes() const noexcept
    {
        return 0; // return values_.capacity()*sizeof(T);
    }

    /***************** End of Implementation Section ******************/

} // namespace CoGaDB
