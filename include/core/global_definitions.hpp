#pragma once

#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace CoGaDB
{

    using ColumnType = std::variant<std::monostate, int, float, std::string, bool>;

    template<typename T>
    concept IColumnType = requires(T t) {
        std::is_same_v<T, bool> || std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::string>;
    };
    /**
     * @brief Possible attribute types supported by the system
     */
    enum class AttributeType
    {
        INT = 1,
        FLOAT,
        VARCHAR,
        BOOLEAN
    };

    template<IColumnType T> struct ValueAttributeTypeMapper {
        static constexpr AttributeType getType() {
            if constexpr (std::is_same_v<T, int>)
                return AttributeType::INT;
            if constexpr (std::is_same_v<T, float>)
                return AttributeType::FLOAT;
            if constexpr (std::is_same_v<T, bool>)
                return AttributeType::BOOLEAN;
            if constexpr (std::is_same_v<T, std::string>)
                return AttributeType::VARCHAR;
        }

        AttributeType attribute_type = getType();
    };

    enum ValueComparator
    {
        LESSER,
        GREATER,
        EQUAL
    };

    enum SortOrder
    {
        ASCENDING,
        DESCENDING
    };

    enum DebugMode
    {
        quiet = 1,
        verbose = 0,
        debug = 0,
        print_time_measurement = 0
    };

    /**
     * @brief The Tuple Identifier (TID) is the unique,numeric identifier of a tuple in a relation
     */
    using TID = std::size_t;

    /**
     * @brief a PositionList is an STL vector of TID values
     */
    using PositionList = std::vector<TID>;

    /** @brief a PositionListPair is an STL pair consisting of two PositionList objects
     *  @details This type is returned by binary operators, e.g., joins
     *  */
    using PositionListPair = std::pair<PositionList, PositionList>;

} // namespace CoGaDB
