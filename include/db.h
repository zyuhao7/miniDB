#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "table.h"

/**
 * @brief 简易的内存型 SQL 数据库实现
 * @date 2025/9/9
 * @author moyuh
 *
 * sqlDB 提供了基本的关系型数据库操作，包括：
 * - 创建带列类型的表
 * - 插入、查询、更新、删除数据
 * - 增删列
 * - 聚合函数 (sum, avg, min, max, count)
 * - 保存和加载所有表
 *
 * 内部通过 `unordered_map<std::string, Table>` 存储多个表。
 */
class sqlDB
{
public:
    /**
     * @brief 创建带列类型的表
     * @param name 表名
     * @param cols 列名和类型组成的向量
     */
    void createTableWithTypes(std::string &name, std::vector<Column> &cols);

    /**
     * @brief 向指定表插入数据
     * @param name 表名
     * @param values 插入的值（顺序与列对应）
     * @param cols 指定插入的列名（可选，用于部分列插入）
     */
    void insertInto(std::string &name, const std::vector<std::string> &values, const std::vector<std::string> &cols);

    /**
     * @brief 查询表中的所有数据
     * @param name 表名
     * @param whereCol 条件列名（默认空表示不筛选）
     * @param whereVal 条件值（与 whereCol 搭配使用）
     * @param orderBy 排序列名（默认空表示不排序）
     * @param desc 是否降序（默认 false 升序）
     * @param limit 限制返回行数（默认 -1 表示无限制）
     */
    void selectAll(const std::string &name, const std::string &whereCol = "",
                   const std::string &whereVal = "", const std::string &orderBy = "",
                   bool desc = false, int limit = -1);

    /**
     * @brief 更新表中满足条件的行
     * @param name 表名
     * @param targetCol 需要更新的列名
     * @param newVal 更新后的新值
     * @param whereCol 条件列名
     * @param whereVal 条件值
     */
    void update(const std::string &name, const std::string &targetCol, const std::string &newVal,
                const std::string &whereCol, const std::string &whereVal);

    /**
     * @brief 删除表中满足条件的行
     * @param name 表名
     * @param whereCol 条件列名
     * @param whereVal 条件值
     */
    void deleteRows(const std::string &name, const std::string &whereCol, const std::string &whereVal);

    /**
     * @brief 保存所有表到文件
     *
     * 文件名可通过内部约定自动生成，通常与表名关联。
     */
    void saveAll();

    /**
     * @brief 从文件加载多个表
     * @param tableNames 需要加载的表名列表
     */
    void loadAll(const std::vector<std::string> &tableNames);

    /**
     * @brief 向表中添加新列
     * @param tableName 表名
     * @param col 新增的列定义
     */
    void addColumn(const std::string &tableName, const Column &col);

    /**
     * @brief 删除表中的某列
     * @param tableName 表名
     * @param colName 列名
     */
    void dropColumn(const std::string &tableName, const std::string &colName);

    /**
     * @brief 对指定列进行聚合运算
     * @param name 表名
     * @param func 聚合函数名称 (sum, avg, min, max, count)
     * @param col 目标列名
     */
    void aggregate(const std::string &name, const std::string &func, std::string &col);

    /**
     * @brief 列出当前数据库中的所有表名
     * @return 表名列表
     */
    std::vector<std::string> listTables() const;

private:
    std::unordered_map<std::string, Table> tables; ///< 内部存储的表集合（键为表名）
};