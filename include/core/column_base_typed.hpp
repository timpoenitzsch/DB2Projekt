
#pragma once

#include <algorithm>
#include <any>
#include <cassert>
#include <core/base_column.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>

//#include <core/column.hpp>

/*! \brief The global namespace of the programming tasks, to avoid name clashes with other libraries.*/
namespace CoGaDB
{

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

    template<class T>
    class ColumnBaseTyped : public ColumnBase
    {
      public:
        using value_type = T;
        // typedef std::shared_ptr<ColumnBaseTyped> ColumnPtr;
        /***************** constructors and destructor *****************/
        ColumnBaseTyped(const std::string &name, AttributeType db_type);
        ~ColumnBaseTyped() override;

        bool insert(const ColumnType &new_Value) override = 0;
        virtual bool insert(const T &new_Value) = 0;
        bool update(TID tid, const ColumnType &new_value) override = 0;
        bool update(PositionList &tid, const ColumnType &new_value) override = 0;

        bool remove(TID tid) override = 0;
        // assumes tid list is sorted ascending
        bool remove(PositionList &tid) override = 0;
        bool clearContent() override = 0;

        ColumnType get(TID tid) override = 0;
        // virtual const std::any* const getRawData()=0;
        void print() const noexcept override = 0;
        [[nodiscard]] size_t size() const noexcept override = 0;
        [[nodiscard]] unsigned int getSizeinBytes() const noexcept override = 0;

        [[nodiscard]] std::unique_ptr<ColumnBase> copy() const override = 0;
        /***************** relational operations on Columns which return lookup tables *****************/
        PositionList sort(SortOrder order) override;
        PositionList selection(const ColumnType &value_for_comparison, ValueComparator comp) override;
        PositionList parallel_selection(const ColumnType &value_for_comparison,
                                        ValueComparator comp,
                                        unsigned int number_of_threads) override;
        // join algorithms
        PositionListPair hash_join(ColumnBase &join_column) override;
        PositionListPair sort_merge_join(ColumnBase &join_column) override;
        PositionListPair nested_loop_join(ColumnBase &join_column) override;

        bool add(const ColumnType &new_Value) override;
        // vector addition between columns
        bool add(ColumnBase &join_column) override;

        bool minus(const ColumnType &new_Value) override;
        bool minus(ColumnBase &join_column) override;

        bool multiply(const ColumnType &new_Value) override;
        bool multiply(ColumnBase &join_column) override;

        bool division(const ColumnType &new_Value) override;
        bool division(ColumnBase &join_column) override;

        // template <typename U, typename BinaryOperator>
        // std::pair<ColumnPtr,ColumnPtr> aggregate_by_keys(ColumnBaseTyped<U>* keys, BinaryOperator binary_op) const;

        void store(const std::string &path) override = 0;
        void load(const std::string &path) override = 0;
        [[nodiscard]] bool isMaterialized() const noexcept override = 0;
        [[nodiscard]] bool isCompressed() const noexcept override = 0;
        /*! \brief defines operator[] for this class, which enables the user to thread all typed columns as arrays.
         * \details Note that this method is pure virtual, so it has to be defined in a derived class.
         * \return a reference to the value at position index
         * */
        virtual T &operator[](int index) = 0;
        inline bool operator==(const ColumnBaseTyped<T> &column) const;
    };

    template<class T>
    ColumnBaseTyped<T>::ColumnBaseTyped(const std::string &name, AttributeType db_type) : ColumnBase(name, db_type)
    {
    }

    template<class T>
    ColumnBaseTyped<T>::~ColumnBaseTyped() = default;

    template<class T>
    PositionList ColumnBaseTyped<T>::sort(SortOrder order)
    {
        PositionList ids;
        std::vector<std::pair<T, TID>> v;

        for (unsigned int i = 0; i < this->size(); i++)
        {
            v.push_back(std::pair<T, TID>((*this)[i], i));
        }

        if (order == ASCENDING)
        {
            // tbb::parallel_sort(v.begin(),v.end(),std::less_equal<std::pair<T,TID> >());
            std::stable_sort(v.begin(), v.end(), std::less_equal<std::pair<T, TID>>());
        }
        else if (order == DESCENDING)
        {
            // tbb::parallel_sort(v.begin(),v.end(),std::greater_equal<std::pair<T,TID> >());
            std::stable_sort(v.begin(), v.end(), std::greater_equal<std::pair<T, TID>>());
        }
        else
        {
            std::cout << "FATAL ERROR: ColumnBaseTyped<T>::sort(): Unknown Sorting Order!" << std::endl;
        }

        for (auto &elem : v)
            ids.push_back(elem.second);

        return ids;
    }

    template<class T>
    PositionList ColumnBaseTyped<T>::parallel_selection(const ColumnType &, const ValueComparator, unsigned int)
    {
        PositionList result_tids;

        return result_tids;
    }

    template<class T>
    PositionList ColumnBaseTyped<T>::selection(const ColumnType &value_for_comparison, const ValueComparator comp)
    {
        T value = std::get<T>(value_for_comparison);

        PositionList result_tids;

        if (!quiet)
            std::cout << "Using CPU for Selection..." << std::endl;
        for (TID i = 0; i < this->size(); i++)
        {
            // std::any value = column->get(i);
            // val = values_[i];

            if (comp == EQUAL)
            {
                if (value == (*this)[i])
                {
                    // result_table->insert(this->fetchTuple(i));
                    result_tids.push_back(i);
                }
            }
            else if (comp == LESSER)
            {
                if ((*this)[i] < value)
                {
                    // result_table->insert(this->fetchTuple(i));
                    result_tids.push_back(i);
                }
            }
            else if (comp == GREATER)
            {
                if ((*this)[i] > value)
                {
                    result_tids.push_back(i);
                    // result_table->insert(this->fetchTuple(i));
                }
            }
            else
            {
            }
        }

        //}
        return result_tids;
    }

    template<class T>
    PositionListPair ColumnBaseTyped<T>::hash_join(ColumnBase &join_column_)
    {
        typedef std::unordered_multimap<T, TID, std::hash<T>, std::equal_to<T>> HashTable;

        if (join_column_.getType() != getType())
        {
            std::cerr << "Fatal Error!!! Typemismatch for columns " << this->name_ << " and " << join_column_.getName()
                      << std::endl;
            std::cerr << "File: " << __FILE__ << " Line: " << __LINE__ << std::endl;
            std::abort();
        }

        auto &join_column = reinterpret_cast<ColumnBaseTyped<T> &>(join_column_); // static_cast<IntColumnPtr>(column1);

        PositionListPair join_tids;

        // create hash table
        HashTable hashtable;
        for (unsigned int i = 0; i < this->size(); i++)
            hashtable.insert(std::pair<T, TID>((*this)[i], i));

        // probe larger relation
        for (unsigned int i = 0; i < join_column.size(); i++)
        {
            std::pair<typename HashTable::iterator, typename HashTable::iterator> range =
                hashtable.equal_range(join_column[i]);
            for (typename HashTable::iterator it = range.first; it != range.second; it++)
            {
                if (it->first == join_column[i])
                {
                    join_tids.first.push_back(it->second);
                    join_tids.second.push_back(i);
                    // cout << "match! " << it->second << ", " << i << "	"  << it->first << endl;
                }
            }
        }

        return join_tids;
    }

    template<class Type>
    PositionListPair ColumnBaseTyped<Type>::sort_merge_join(ColumnBase &join_column_)
    {
        if (join_column_.getType() != getType())
        {
            std::cout << "Fatal Error!!! Typemismatch for columns " << this->name_ << " and " << join_column_.getName()
                      << std::endl;
            std::cout << "File: " << __FILE__ << " Line: " << __LINE__ << std::endl;
            abort();
        }

        PositionListPair join_tids;
        return join_tids;
    }

    template<class Type>
    PositionListPair ColumnBaseTyped<Type>::nested_loop_join(ColumnBase &join_column_)
    {
        if (join_column_.getType() != getType())
        {
            std::cout << "Fatal Error!!! Typemismatch for columns " << this->name_ << " and " << join_column_.getName()
                      << std::endl;
            std::cout << "File: " << __FILE__ << " Line: " << __LINE__ << std::endl;
            exit(-1);
        }

        auto &join_column =
            reinterpret_cast<ColumnBaseTyped<Type> &>(join_column_); // static_cast<IntColumnPtr>(column1);

        PositionListPair join_tids;

        for (unsigned int i = 0; i < this->size(); i++)
        {
            for (unsigned int j = 0; j < join_column.size(); j++)
            {
                if ((*this)[i] == join_column[j])
                {
                    if (debug)
                        std::cout << "MATCH: (" << i << "," << j << ")" << std::endl;
                    join_tids.first.push_back(i);
                    join_tids.second.push_back(j);
                }
            }
        }

        return join_tids;
    }

    template<class T>
    bool ColumnBaseTyped<T>::operator==(const ColumnBaseTyped<T> &column) const
    {
        if (this->size() != column.size())
            return false;
        for (unsigned int i = 0; i < this->size(); i++)
        {
            if (const_cast<ColumnBaseTyped<T> &>(*this)[i] != const_cast<ColumnBaseTyped<T> &>(column)[i])
            {
                return false;
            }
        }
        return true;
    }

    template<class Type>
    bool ColumnBaseTyped<Type>::add(const ColumnType &new_value)
    {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        auto value = std::get<Type>(new_value);
        // std::transform(myvec.begin(), myvec.end(), myvec.begin(),
        // bind2nd(std::plus<double>(), 1.0));
        for (unsigned int i = 0; i < this->size(); i++)
        {
            this->operator[](i) += value;
        }
        return true;
    }

    template<class Type>
    bool ColumnBaseTyped<Type>::add(ColumnBase &column)
    {
        // std::transform ( first, first+5, second, results, std::plus<int>() );
        auto &typed_column = dynamic_cast<ColumnBaseTyped<Type> &>(column);

        for (unsigned int i = 0; i < this->size(); i++)
        {
            this->operator[](i) += typed_column[i];
        }
        return true;
    }

    template<class Type>
    bool ColumnBaseTyped<Type>::minus(const ColumnType &new_value)
    {
        // shared_pointer_namespace::shared_ptr<ColumnBaseTyped<Type> > typed_column =
        // shared_pointer_namespace::static_pointer_cast<ColumnBaseTyped<Type> >(column);
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        auto value = std::get<Type>(new_value);
        for (unsigned int i = 0; i < this->size(); i++)
        {
            this->operator[](i) -= value;
        }
        return true;
    }

    template<class Type>
    bool ColumnBaseTyped<Type>::minus(ColumnBase &column)
    {
        // std::transform ( first, first+5, second, results, std::plus<int>() );
        auto &typed_column = reinterpret_cast<ColumnBaseTyped<Type> &>(column);

        for (unsigned int i = 0; i < this->size(); i++)
        {
            this->operator[](i) -= typed_column[i];
        }
        return true;
    }

    template<class Type>
    bool ColumnBaseTyped<Type>::multiply(const ColumnType &new_value)
    {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        Type value = std::any_cast<Type>(new_value);
        for (unsigned int i = 0; i < this->size(); i++)
        {
            this->operator[](i) *= value;
        }
        return true;
    }

    template<class Type>
    bool ColumnBaseTyped<Type>::multiply(ColumnBase &column)
    {
        // std::transform ( first, first+5, second, results, std::plus<int>() );
        auto &typed_column = dynamic_cast<ColumnBaseTyped<Type> &>(column);

        for (unsigned int i = 0; i < this->size(); i++)
        {
            this->operator[](i) *= typed_column[i];
        }
        return true;
    }

    template<class Type>
    bool ColumnBaseTyped<Type>::division(const ColumnType &new_value)
    {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        Type value = std::any_cast<Type>(new_value);
        // check that we do not devide by zero
        if (value == 0)
            return false;
        for (unsigned int i = 0; i < this->size(); i++)
        {
            this->operator[](i) /= value;
        }
        return true;
    }

    template<class Type>
    bool ColumnBaseTyped<Type>::division(ColumnBase &column)
    {
        // std::transform ( first, first+5, second, results, std::plus<int>() );
        auto &typed_column = reinterpret_cast<ColumnBaseTyped<Type> &>(column);

        for (unsigned int i = 0; i < this->size(); i++)
        {
            this->operator[](i) /= typed_column[i];
        }
        return true;
    }

    // total tempalte specializations, because numeric computations are undefined on strings
    template<>
    inline bool ColumnBaseTyped<std::string>::add(const ColumnType &)
    {
        return false;
    }
    template<>
    inline bool ColumnBaseTyped<std::string>::add(ColumnBase &)
    {
        return false;
    }

    template<>
    inline bool ColumnBaseTyped<std::string>::minus(const ColumnType &)
    {
        return false;
    }
    template<>
    inline bool ColumnBaseTyped<std::string>::minus(ColumnBase &)
    {
        return false;
    }

    template<>
    inline bool ColumnBaseTyped<std::string>::multiply(const ColumnType &)
    {
        return false;
    }
    template<>
    inline bool ColumnBaseTyped<std::string>::multiply(ColumnBase &)
    {
        return false;
    }

    template<>
    inline bool ColumnBaseTyped<std::string>::division(const ColumnType &)
    {
        return false;
    }
    template<>
    inline bool ColumnBaseTyped<std::string>::division(ColumnBase &)
    {
        return false;
    }

}; // namespace CoGaDB
