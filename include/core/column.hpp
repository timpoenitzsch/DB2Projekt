#pragma once

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <core/column_base_typed.hpp>
#include <fstream>
#include <iostream>

namespace CoGaDB
{

    template<typename T>
    class Column : public ColumnBaseTyped<T>
    {
      public:
        /***************** constructors and destructor *****************/
        Column(const std::string &name, AttributeType db_type);
        // Column(const Column& column);
        ~Column() override;

        bool insert(const ColumnType &new_value) override;
        bool insert(const T &new_value) override;
        template<typename InputIterator>
        bool insert(InputIterator first, InputIterator last);

        bool update(TID tid, const ColumnType &new_value) override;
        bool update(PositionList &tid, const ColumnType &new_value) override;

        bool remove(TID tid) override;
        // assumes tid list is sorted ascending
        bool remove(PositionList &tid) override;
        bool clearContent() override;

        ColumnType get(TID tid) override;
        void print() const noexcept override;
        [[nodiscard]] size_t size() const noexcept override;
        [[nodiscard]] unsigned int getSizeinBytes() const noexcept override;

        [[nodiscard]] std::unique_ptr<ColumnBase> copy() const override;

        void store(const std::string &path) override;
        void load(const std::string &path) override;
        [[nodiscard]] bool isMaterialized() const noexcept override;
        [[nodiscard]] bool isCompressed() const noexcept override;

        template<class Archive>
        [[maybe_unused]] void serialize(Archive &archive)
        {
            archive(values_); // serialize things by passing them to the archive
        }

        T &operator[](int index) override;

        [[maybe_unused]] std::vector<T> &getContent();

      private:
        struct Type_TID_Comparator
        {
            inline bool operator()(std::pair<T, TID> i, std::pair<T, TID> j)
            {
                return (i.first < j.first);
            }
        } type_tid_comparator;

        /*! values*/
        std::vector<T> values_;
    };

    /***************** Start of Implementation Section ******************/

    template<class T>
    Column<T>::Column(const std::string &name, AttributeType db_type) :
        ColumnBaseTyped<T>(name, db_type), type_tid_comparator(), values_()
    {
    }

    //	template<class T>
    //	Column<T>::Column(const Column& column) : this->db_type_(column->db_type_), this->values_(column->values_){
    //
    //	}

    template<class T>
    Column<T>::~Column() = default;

    template<class T>
    [[maybe_unused]] std::vector<T> &Column<T>::getContent()
    {
        return values_;
    }

    template<class T>
    bool Column<T>::insert(const ColumnType &new_value)
    {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        T value = std::get<T>(new_value);
        values_.push_back(value);
        return true;
    }

    template<class T>
    bool Column<T>::insert(const T &new_value)
    {
        values_.push_back(new_value);
        return true;
    }

    template<typename T>
    template<typename InputIterator>
    bool Column<T>::insert(InputIterator first, InputIterator last)
    {
        this->values_.insert(this->values_.end(), first, last);
        return true;
    }

    template<class T>
    bool Column<T>::update(TID tid, const ColumnType &new_value)
    {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        T value = std::get<T>(new_value);
        values_[tid] = value;
        return true;
    }

    template<class T>
    bool Column<T>::update(PositionList &tids, const ColumnType &new_value)
    {
        if (std::holds_alternative<std::monostate>(new_value))
            return false;

        T value = std::get<T>(new_value);
        for (unsigned int tid : tids)
        {
            values_[tid] = value;
        }
        return true;
    }

    template<class T>
    bool Column<T>::remove(TID tid)
    {
        values_.erase(values_.begin() + tid);
        return true;
    }

    template<class T>
    bool Column<T>::remove(PositionList &tids)
    {
        // test whether tid list has at least one element, if not, return with error
        if (tids.empty())
            return false;

        for (auto rit = tids.rbegin(); rit != tids.rend(); ++rit)
            values_.erase(values_.begin() + (*rit));

        return true;
    }

    template<class T>
    bool Column<T>::clearContent()
    {
        values_.clear();
        return true;
    }

    template<class T>
    ColumnType Column<T>::get(TID tid)
    {
        if (tid < values_.size())
            return values_[tid];
        else
        {
            std::cerr << "fatal Error!!! Invalid TID!!! Attribute: " << this->name_ << " TID: " << tid << std::endl;
        }
        return {};
    }

    template<class T>
    void Column<T>::print() const noexcept
    {
        std::cout << "| " << this->name_ << " |" << std::endl;
        std::cout << "________________________" << std::endl;
        for (unsigned int i = 0; i < values_.size(); i++)
        {
            std::cout << "| " << values_[i] << " |" << std::endl;
        }
    }
    template<class T>
    size_t Column<T>::size() const noexcept
    {
        return values_.size();
    }
    template<class T>
    std::unique_ptr<ColumnBase> Column<T>::copy() const
    {
        return std::make_unique<Column<T>>(*this);
    }
    /***************** relational operations on Columns which return lookup tables *****************/
    //	template<class T>
    //	const std::vector<TID> Column<T>::sort(const ComputeDevice comp_dev) const {

    //		return std::vector<TID>();
    //	}

    //	template<class T>
    //	const std::vector<TID> Column<T>::selection(const std::any& value_for_comparison, const ValueComparator comp,
    // const ComputeDevice comp_dev) const {

    //		return std::vector<TID>();
    //	}
    //	//join algorithms
    //	template<class T>
    //	const std::vector<TID_Pair> Column<T>::sort_merge_join(ColumnPtr join_Column, const ComputeDevice comp_dev)
    // const{

    //		return std::vector<TID_Pair>();
    //	}
    //	template<class T>
    //	const std::vector<TID_Pair> Column<T>::nested_loop_join(ColumnPtr join_Column, const ComputeDevice comp_dev)
    // const{

    //		return std::vector<TID_Pair>();
    //	}
    template<class T>
    void Column<T>::load(const std::string &path_)
    {
        std::string path(path_);
        // std::cout << "Loading column '" << this->name_ << "' from path '" << path << "'..." << std::endl;
        // string path("data/");
        path += "/";
        path += this->name_;

        // std::cout << "Opening File '" << path << "'..." << std::endl;
        std::ifstream infile(path.c_str(), std::ios_base::binary | std::ios_base::in);
        cereal::PortableBinaryInputArchive ia(infile);
        ia(*this);
    }

    template<class T>
    void Column<T>::store(const std::string &path_)
    {
        std::string path(path_);
        // std::cout << "Loading column '" << this->name_ << "' from path '" << path << "'..." << std::endl;
        // string path("data/");
        path += "/";
        path += this->name_;

        // std::cout << "Opening File '" << path << "'..." << std::endl;
        std::ofstream outfile(path.c_str(), std::ios_base::binary | std::ios_base::out);
        cereal::PortableBinaryOutputArchive oarchive(outfile); // Create an output archive
        oarchive(*this);
    }

    template<class T>
    bool Column<T>::isMaterialized() const noexcept
    {
        return true;
    }

    template<class T>
    bool Column<T>::isCompressed() const noexcept
    {
        return false;
    }

    template<class T>
    T &Column<T>::operator[](const int index)
    {
        return values_[index];
    }

    template<class T>
    unsigned int Column<T>::getSizeinBytes() const noexcept
    {
        return values_.capacity() * sizeof(T);
    }

    // total template specialization
    template<>
    inline unsigned int Column<std::string>::getSizeinBytes() const noexcept
    {
        unsigned int size_in_bytes = 0;
        for (const auto &value : values_)
        {
            size_in_bytes += value.capacity();
        }
        // return values_.size()*sizeof(T);
        return size_in_bytes;
    }

    /***************** End of Implementation Section ******************/

} // namespace CoGaDB
