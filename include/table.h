#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cstdlib>    // getenv
#include <sys/stat.h> // mkdir
#include <sys/types.h>
#include <cerrno>
#include <cstring>
#include "types.h"

/**
 * @brief 表示一个列(Column)，包含列名和数据类型
 */
struct Column
{
    std::string name; ///< 列名
    DataType type;    ///< 列的数据类型
};

/**
 * @brief 表示一行(Row)，存储为字符串向量
 */
struct Row
{
    std::vector<std::string> values; ///< 一行中的各个单元格值
};

/**
 * @brief 表格数据结构，包含列定义和行数据
 *
 * Table 用于表示一个简单的表格数据结构：
 * - 包含列(Column)定义（列名、数据类型）
 * - 包含行(Row)数据（存储为字符串向量）
 * - 提供列索引查询、文件保存与加载功能
 */
struct Table
{
    std::vector<Column> columns; ///< 表的列集合
    std::vector<Row> rows;       ///< 表的行集合

    /**
     * @brief 根据列名获取列索引
     * @param colName 列名
     * @return 列索引，如果未找到则返回 -1
     */
    int getColumnIndex(const std::string &colName) const;

    /**
     * @brief 将表格数据保存到文件
     * @param filename 文件名
     */
    void saveToFile(const std::string &filename);

    /**
     * @brief 从文件加载表格数据
     * @param filename 文件名
     */
    void loadFromFile(const std::string &filename);
};
