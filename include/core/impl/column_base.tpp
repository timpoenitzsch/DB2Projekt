#pragma once

namespace CoGaDB {

    template<template<typename> class D, IColumnType T>
    PositionList ColumnBase<D, T>::sort(SortOrder order) {
        PositionList ids;
        std::vector<std::pair<value_type, TID>> v;

        for (unsigned int i = 0; i < this->size(); i++) {
            v.push_back(std::pair<value_type, TID>((*this)[i], i));
        }

        if (order == ASCENDING) {
            std::stable_sort(v.begin(), v.end(), std::less_equal<std::pair<value_type, TID>>());
        } else if (order == DESCENDING) {
            std::stable_sort(v.begin(), v.end(), std::greater_equal<std::pair<value_type, TID>>());
        } else {
            std::cout << "FATAL ERROR: ColumnBaseTyped<T>::sort(): Unknown Sorting Order!" << std::endl;
        }

        for (auto &elem: v)
            ids.push_back(elem.second);

        return ids;
    }

    template<template<typename> class D, IColumnType T>
    PositionList ColumnBase<D, T>::selection(const ColumnType &value_for_comparison, ValueComparator comp) {
        value_type value = std::get<value_type>(value_for_comparison);

        PositionList result_tids;

        if (!quiet)
            std::cout << "Using CPU for Selection..." << std::endl;
        for (TID i = 0; i < this->size(); i++) {
            // std::any value = column->get(i);
            // val = values_[i];

            if (comp == EQUAL) {
                if (value == (*this)[i]) {
                    // result_table->insert(this->fetchTuple(i));
                    result_tids.push_back(i);
                }
            } else if (comp == LESSER) {
                if ((*this)[i] < value) {
                    // result_table->insert(this->fetchTuple(i));
                    result_tids.push_back(i);
                }
            } else if (comp == GREATER) {
                if ((*this)[i] > value) {
                    result_tids.push_back(i);
                    // result_table->insert(this->fetchTuple(i));
                }
            } else {
            }
        }

        //}
        return result_tids;
    }

    template<template<typename> class D, IColumnType T>
    PositionList ColumnBase<D, T>::parallel_selection(const ColumnType &, ValueComparator, unsigned int) {
        PositionList result_tids;

        return result_tids;
    }

    template<template<typename> class D, IColumnType T>
    template<IColumn<T> C>
    PositionListPair ColumnBase<D, T>::hash_join(C &join_column) {
        typedef std::unordered_multimap<value_type, TID, std::hash<value_type>, std::equal_to<value_type>> HashTable;

        if (join_column.getType() != getType()) {
            std::cerr << "Fatal Error!!! Type mismatch for columns " << this->name_ << " and " << join_column.getName()
                      << std::endl;
            std::cerr << "File: " << __FILE__ << " Line: " << __LINE__ << std::endl;
            std::abort();
        }

        PositionListPair join_tids;

        // create hash table
        HashTable hashtable;
        for (unsigned int i = 0; i < this->size(); i++)
            hashtable.insert(std::pair<value_type, TID>((*this)[i], i));

        // probe larger relation
        for (unsigned int i = 0; i < join_column.size(); i++) {
            std::pair<typename HashTable::iterator, typename HashTable::iterator> range =
                    hashtable.equal_range(join_column[i]);
            for (typename HashTable::iterator it = range.first; it != range.second; it++) {
                if (it->first == join_column[i]) {
                    join_tids.first.push_back(it->second);
                    join_tids.second.push_back(i);
                    // cout << "match! " << it->second << ", " << i << "	"  << it->first << endl;
                }
            }
        }

        return join_tids;
    }

    template<template<typename> class D, IColumnType T>
    template<IColumn<T> C>
    PositionListPair ColumnBase<D, T>::sort_merge_join(C &join_column) {
        if (join_column.getType() != getType()) {
            std::cout << "Fatal Error!!! Type mismatch for columns " << this->name_ << " and " << join_column.getName()
                      << std::endl;
            std::cout << "File: " << __FILE__ << " Line: " << __LINE__ << std::endl;
            abort();
        }
        PositionListPair join_tids;
        return join_tids;
    }

    template<template<typename> class D, IColumnType T>
    template<IColumn<T> C>
    PositionListPair ColumnBase<D, T>::nested_loop_join(C join_column) {
        if (join_column.getType() != getType()) {
            std::cout << "Fatal Error!!! Type mismatch for columns " << this->name_ << " and " << join_column.getName()
                      << std::endl;
            std::cout << "File: " << __FILE__ << " Line: " << __LINE__ << std::endl;
            exit(-1);
        }

        PositionListPair join_tids;

        for (unsigned int i = 0; i < this->size(); i++) {
            for (unsigned int j = 0; j < join_column.size(); j++) {
                if ((*this)[i] == join_column[j]) {
                    if (debug)
                        std::cout << "MATCH: (" << i << "," << j << ")" << std::endl;
                    join_tids.first.push_back(i);
                    join_tids.second.push_back(j);
                }
            }
        }

        return join_tids;
    }

    template<template<typename> class D, IColumnType T>
    bool ColumnBase<D, T>::add(const ColumnType &new_value) {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        auto value = std::get<value_type>(new_value);

        for (unsigned int i = 0; i < this->size(); i++) {
            this->update(i, this->operator[](i) + value);
        }
        return true;
    }

    template<template<typename> class D, IColumnType T>
    template<IColumn<T> C>
    bool ColumnBase<D, T>::add(C &column) {
        for (unsigned int i = 0; i < this->size(); i++) {
            this->update(i, this->operator[](i) + column[i]);
        }
        return true;
    }

    template<template<typename> class D, IColumnType T>
    bool ColumnBase<D, T>::minus(const ColumnType &new_value) {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;
        //TODO: does not work for string?
        auto value = std::get<value_type>(new_value);
        for (unsigned int i = 0; i < this->size(); i++) {
            this->update(i, this->operator[](i) - value);
        }
        return true;
    }

    template<template<typename> class D, IColumnType T>
    template<IColumn<T> C>
    bool ColumnBase<D, T>::minus(C &join_column) {
        // std::transform ( first, first+5, second, results, std::plus<int>() );
        for (unsigned int i = 0; i < this->size(); i++) {
            this->update(i, this->operator[](i) - join_column[i]);
        }
        return true;
    }

    template<template<typename> class D, IColumnType T>
    bool ColumnBase<D, T>::multiply(const ColumnType &new_value) {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        value_type value = std::any_cast<value_type>(new_value);
        for (unsigned int i = 0; i < this->size(); i++) {
            auto tmp = this->operator[](i) * value;
            this->update(i, tmp);
        }
        return true;
    }

    template<template<typename> class D, IColumnType T>
    template<IColumn<T> C>
    bool ColumnBase<D, T>::multiply(C &join_column) {
        // std::transform ( first, first+5, second, results, std::plus<int>() );
        for (unsigned int i = 0; i < this->size(); i++) {
            auto tmp = this->operator[](i) * join_column[i];
            this->update(i, tmp);
        }
        return true;
    }

    template<template<typename> class D, IColumnType T>
    bool ColumnBase<D, T>::division(const ColumnType &new_value) {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        auto value = std::get<value_type>(new_value);
        // check that we do not divide by zero
        if (value == 0)
            return false;
        for (unsigned int i = 0; i < this->size(); i++) {
            auto val = this->operator[](i) / value;
            this->update(i, val);
        }
        return true;
    }

    template<template<typename> class D, IColumnType T>
    template<IColumn<T> C>
    bool ColumnBase<D, T>::division(C &column) {
        // std::transform ( first, first+5, second, results, std::plus<int>() );

        for (unsigned int i = 0; i < this->size(); i++) {
            auto val = this->operator[](i) / column[i];
            this->update(i, val);
        }
        return true;
    }

    template<template<typename> class D, IColumnType T>
    void ColumnBase<D, T>::store(const std::string &path_) {
        std::string path(path_);
        path += this->name_;

        std::ofstream outfile(path.c_str(), std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
        assert(outfile.is_open());
        cereal::PortableBinaryOutputArchive oarchive(outfile);// Create an output archive
        oarchive(self());
    }

    template<template<typename> class D, IColumnType T>
    void ColumnBase<D, T>::load(const std::string &path_) {
        std::string path(path_);
        path += this->name_;

        std::ifstream infile(path.c_str(), std::ifstream::binary | std::ifstream::in);
        cereal::PortableBinaryInputArchive ia(infile);
        ia(self());
    }

    template<template<typename> class D, IColumnType T>
    bool ColumnBase<D, T>::operator==(const ColumnBase<D, T>::Derived &column) const {
        if (self().size() != column.size())
            return false;
        for (unsigned int i = 0; i < self().size(); i++) {
            if (self().get(i) != column.get(i)) {//why const-cast?
                return false;
            }
        }
        return true;
    }


}// namespace CoGaDB
