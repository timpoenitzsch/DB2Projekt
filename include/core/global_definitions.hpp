#pragma once

#include <any>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace CoGaDB
{

    enum AttributeType
    {
        INT,
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

    using Tuple = std::vector<std::any>;

}; // namespace CoGaDB
