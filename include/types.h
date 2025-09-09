#pragma once
#include <string>
#include <algorithm>
#include <stdexcept>

enum class DataType
{
    INT,
    TEXT,
    FLOAT,
    DOUBLE,
    DATE,
    BOOL,
    VARCHAR
};

DataType parseType(const std::string &typeStr);