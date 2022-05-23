
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
    class DictionaryCompressedColumn final : public CompressedColumn<T> {
    public:
        /***************** constructors and destructor *****************/
        explicit DictionaryCompressedColumn(const std::string &name);

        ~DictionaryCompressedColumn() final;

        void insert(const ColumnType &new_Value) final;

        void insert(const T &new_value) final;

        template<typename InputIterator>
        void insert(InputIterator first, InputIterator last);

        void update(TID tid, const ColumnType &new_value) final;

        void update(PositionList &tid, const ColumnType &new_value) final;

        void remove(TID tid) final;

        // assumes tid list is sorted ascending
        void remove(PositionList &tid) final;

        void clearContent() final;

        ColumnType get(TID tid) final;

        // virtual const std::any* const getRawData()=0;
        std::string print() const noexcept final;

        [[nodiscard]] size_t size() const noexcept final;

        [[nodiscard]] size_t getSizeInBytes() const noexcept final;

        [[nodiscard]] virtual std::unique_ptr<ColumnBase> copy() const;

        void store(const std::string &path) final;
        void load(const std::string &path) final;

        T operator[](int index) final;

        /**
         * @brief Serialization method called by Cereal. Implement this method in your compressed columns to get serialization working.
         */
        template<class Archive>
        void serialize(Archive &archive) {
            //TODO: implement
            archive();// serialize things by passing them to the archive
        }
    };

    /***************** Start of Implementation Section ******************/

    template<class T>
    DictionaryCompressedColumn<T>::DictionaryCompressedColumn(const std::string &name) : CompressedColumn<T>(name) {
        //TODO: implement
    }

    template<class T>
    DictionaryCompressedColumn<T>::~DictionaryCompressedColumn() = default;

    template<class T>
    void DictionaryCompressedColumn<T>::insert(const ColumnType &) {
        //TODO: implement
    }

    template<class T>
    void DictionaryCompressedColumn<T>::insert(const T &) {
        //TODO: implement
    }

    template<typename T>
    template<typename InputIterator>
    void DictionaryCompressedColumn<T>::insert(InputIterator, InputIterator) {
        //TODO: implement
    }

    template<class T>
    ColumnType DictionaryCompressedColumn<T>::get(TID) {
        //TODO: implement
        return {};
    }

    template<class T>
    std::string DictionaryCompressedColumn<T>::print() const noexcept {
        //TODO: implement
        return {};
    }

    template<class T>
    size_t DictionaryCompressedColumn<T>::size() const noexcept {
        //TODO: implement
        return 0;
    }

    template<class T>
    std::unique_ptr<ColumnBase> DictionaryCompressedColumn<T>::copy() const {
        //TODO: implement
        return {};
    }

    template<class T>
    void DictionaryCompressedColumn<T>::update(TID, const ColumnType &) {
        //TODO: implement
    }

    template<class T>
    void DictionaryCompressedColumn<T>::update(PositionList &, const ColumnType &) {
        //TODO: implement
    }

    template<class T>
    void DictionaryCompressedColumn<T>::remove(TID) {
        //TODO: implement
    }

    template<class T>
    void DictionaryCompressedColumn<T>::remove(PositionList &) {
        //TODO: implement
    }

    template<class T>
    void DictionaryCompressedColumn<T>::clearContent() {
        //TODO: implement
    }

    template<class T>
    void DictionaryCompressedColumn<T>::store(const std::string &) {
        //TODO: implement
    }

    template<class T>
    void DictionaryCompressedColumn<T>::load(const std::string &) {
        //TODO: implement
    }


    template<class T>
    T DictionaryCompressedColumn<T>::operator[](const int) {
        //TODO: implement
        return {};
    }

    template<class T>
    size_t DictionaryCompressedColumn<T>::getSizeInBytes() const noexcept {
        //TODO: implement
        return 0;
    }

    /***************** End of Implementation Section ******************/

}// namespace CoGaDB
