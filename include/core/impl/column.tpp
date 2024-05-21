#pragma once
namespace CoGaDB {
    template<IColumnType T>
    Column<T>::Column(std::string name) : ColumnBase<Column, T>(name), type_tid_comparator(), values_() {}


    template<IColumnType T>
    void Column<T>::insert_impl(const ColumnType &new_value) {
        //will throw if types do not match
        auto value = std::get<value_type>(new_value);
        values_.push_back(value);
    }

    template<IColumnType T>
    void Column<T>::insert_impl(const value_type &new_value) {
        values_.push_back(new_value);
    }

    template<IColumnType T>
    template<std::input_iterator InputIterator, std::sentinel_for<InputIterator> Sentinel>
    void Column<T>::insert_impl(const InputIterator &first, const Sentinel &last) {
        this->values_.insert(this->values_.end(), first, last);
    }

    template<IColumnType T>
    void Column<T>::update_impl(const TID tid, const ColumnType &new_value) {
        //will throw if new_value doesn't hold type T
        auto value = std::get<value_type>(new_value);
        values_[tid] = value;
    }

    template<IColumnType T>
    void Column<T>::update_impl(const PositionList &tids, const ColumnType &new_value) {
        //will throw if new_value doesn't hold type T
        T value = std::get<T>(new_value);
        for (auto tid: tids) {
            values_[tid] = value;
        }
    }

    template<IColumnType T>
    void Column<T>::remove_impl(const TID tid) {
        values_.erase(values_.begin() + tid);
    }

    template<IColumnType T>
    void Column<T>::remove_impl(const PositionList &tids) {
        for (const unsigned long &tid: std::ranges::reverse_view(tids))
            values_.erase(values_.begin() + tid);
    }

    template<IColumnType T>
    void Column<T>::clearContent_impl() {
        values_.clear();
    }

    template<IColumnType T>
    ColumnType Column<T>::get_impl(const TID tid) {
        return values_.at(tid);
    }

    template<IColumnType T>
    ColumnType Column<T>::get_impl(const TID tid) const {
        return values_.at(tid);
    }

    template<IColumnType T>
    std::string Column<T>::print_impl() const noexcept {
        return std::accumulate(values_.cbegin(), values_.cend(), "| " + this->getName() + " |\n________________________\n",
                               [](std::string acc, const T &cur) {
                                   if constexpr (std::is_same_v<std::string, T>)
                                       return std::move(acc) + "| " + cur + " |\n";
                                   else
                                       return std::move(acc) + "| " + std::to_string(cur) + " |\n";
                               });
    }

    template<IColumnType T>
    size_t Column<T>::getSizeInBytes_impl() const noexcept {
        if constexpr (std::is_same_v<value_type, std::string>) {
            return std::accumulate(values_.cbegin(), values_.cend(), 0,
                                   [](size_t acc, const std::string &val) { return acc + val.size(); });
        }

        return values_.capacity() * sizeof(T);
    }

    template<IColumnType T>
    size_t Column<T>::size_impl() const noexcept {
        return values_.size();
    }

    template<IColumnType T>
    std::unique_ptr<Column<T>> Column<T>::copy_impl() const {
        return std::make_unique<Column<T>>(*this);
    }

    template<IColumnType T>
    template<class Archive>
    void Column<T>::serialize(Archive &archive) {
        archive(values_);// serialize things by passing them to the archive
    }
}