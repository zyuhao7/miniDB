#pragma once
#include <string>
#include <algorithm>
#include <stdexcept>
/**
 * @brief 数据类型枚举类
 * 
 * 定义了系统中支持的各种数据类型，用于标识字段或变量的数据类型。
 */
enum class DataType
{
    INT,      // 整型数据
    TEXT,     // 文本类型
    FLOAT,    // 单精度浮点型
    DOUBLE,   // 双精度浮点型
    DATE,     // 日期类型
    BOOL,     // 布尔类型
    VARCHAR   // 可变长度字符串类型
};

DataType parseType(const std::string &typeStr);