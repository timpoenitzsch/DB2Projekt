#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace CoGaDB
{

    using ColumnType = std::variant<std::monostate,int, float, std::string, bool>;

    enum AttributeType
    {
        INT=1,
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

    using TID_Pair = std::pair<TID, TID>;

    using Attribut = std::pair<AttributeType, std::string>;

    using TableSchema = std::list<Attribut>;

    using Tuple = std::vector<ColumnType>;

}; // namespace CoGaDB
