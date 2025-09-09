#include "types.h"
/**
 *  @brief 解析字符串到枚举类成员
 */
DataType parseType(const std::string &typeStr)
{
    std::string t = typeStr;
    std::transform(t.begin(), t.end(), t.begin(), ::toupper);
    if (t == "INT")
        return DataType::INT;
    if (t == "TEXT")
        return DataType::TEXT;
    if (t == "FLOAT")
        return DataType::FLOAT;
    if (t == "DOUBLE")
        return DataType::DOUBLE;
    if (t == "DATE")
        return DataType::DATE;
    if (t == "VARCHAR")
        return DataType::VARCHAR;
    if (t == "BOOL")
        return DataType::BOOL;

    throw std::invalid_argument("Unknown data type: " + typeStr);
}