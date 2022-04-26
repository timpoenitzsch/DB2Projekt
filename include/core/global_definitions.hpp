#pragma once

#include <string>
#include <variant>
#include <utility>

namespace CoGaDB
{

    using ColumnType = std::variant<std::monostate, int, float, std::string, bool>;

    enum AttributeType
    {
        INT = 1,
        FLOAT,
        VARCHAR,
        BOOLEAN
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

    using TID = unsigned int;
    using Attribut = std::pair<AttributeType, std::string>;

} // namespace CoGaDB
