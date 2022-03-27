#pragma once

#include <core/column_base_typed.hpp>

namespace CoGaDB
{

    /*!
     *
     *
     *  \brief     This class represents a compressed column with type T, is the base class for all compressed typed
     * column classes and allows a uniform handling of compressed columns of a certain type T. \details   This class is
     * indentended to be a base class, so it has a virtual destruktor and pure virtual methods, which need to be
     * implemented in a derived class. \author    Sebastian Breß \version   0.2 \date      2013 \copyright GNU LESSER
     * GENERAL PUBLIC LICENSE - Version 3, http://www.gnu.org/licenses/lgpl-3.0.txt
     */
    template<class T>
    class CompressedColumn : public ColumnBaseTyped<T>
    {
      public:
        /***************** constructors and destructor *****************/
        CompressedColumn(const std::string &name, AttributeType db_type);
        virtual ~CompressedColumn();

        virtual bool insert(const ColumnType &new_Value) = 0;
        virtual bool insert(const T &new_value) = 0;

        virtual bool update(TID tid, const ColumnType &new_value) = 0;
        virtual bool update(PositionList &tid, const ColumnType &new_value) = 0;

        virtual bool remove(TID tid) = 0;
        // assumes tid list is sorted ascending
        virtual bool remove(PositionList &tid) = 0;
        virtual bool clearContent() = 0;

        virtual ColumnType get(TID tid) = 0;

        virtual void print() const noexcept = 0;
        [[nodiscard]] virtual size_t size() const noexcept = 0;
        [[nodiscard]] virtual unsigned int getSizeinBytes() const noexcept = 0;

        [[nodiscard]] virtual std::unique_ptr<ColumnBase> copy() const = 0;

        virtual void store(const std::string &path) = 0;
        virtual void load(const std::string &path) = 0;
        [[nodiscard]] virtual bool isMaterialized() const noexcept;

        [[nodiscard]] virtual bool isCompressed() const noexcept;

        virtual T &operator[](int index) = 0;

        virtual T &operator[](int index) const = 0;
    };

    using CompressedIntegerColumn = CompressedColumn<int>;
    using CompressedFloatColumn = CompressedColumn<float>;
    using CompressedDoubleColumn = CompressedColumn<double>;
    using CompressedStringColumn = CompressedColumn<std::string>;

    /***************** Start of Implementation Section ******************/

    template<class T>
    CompressedColumn<T>::CompressedColumn(const std::string &name, AttributeType db_type) :
        ColumnBaseTyped<T>(name, db_type)
    {
    }

    template<class T>
    CompressedColumn<T>::~CompressedColumn() = default;

    template<class T>
    bool CompressedColumn<T>::isMaterialized() const noexcept
    {
        return false;
    }

    template<class T>
    bool CompressedColumn<T>::isCompressed() const noexcept
    {
        return true;
    }

    /*
        template<class T>
        bool CompressedColumn<T>::insert(const std::any& new_Value){

            return false;
        }

        template<class T>
        const std::any CompressedColumn<T>::get(TID tid){

            return std::any();
        }

        template<class T>
        void CompressedColumn<T>::print() const throw(){

        }
        template<class T>
        size_t CompressedColumn<T>::size() const throw(){

            return 0;
        }
        template<class T>
        const ColumnPtr CompressedColumn<T>::copy() const{

            return ColumnPtr(NULL);
        }

        template<class T>
        bool CompressedColumn<T>::store(const std::string& path){

            return std::vector<TID>();
        }
        template<class T>
        bool CompressedColumn<T>::load(const std::string& path){

            return false;
        }
        template<class T>
        bool CompressedColumn<T>::isMaterialized() const  throw(){

            return false;
        }

        template<class T>
        bool CompressedColumn<T>::isCompressed() const  throw(){
            return true;
        }

        template<class T>
        T& CompressedColumn<T>::operator[](const int index){
            static T t;
            return t;
        }

        template<class T>
        unsigned int CompressedColumn<T>::getSizeinBytes() const throw(){
            return values_.capacity()*sizeof(T);
        }
    */

    /***************** End of Implementation Section ******************/

}; // namespace CoGaDB
