#include "db.h"
#include <iostream>
#include <cctype>
#include <algorithm>
#include <filesystem>
#include <numeric>

/**
 * @brief 去除字符串两端的空白字符，并将字符串转换为小写
 * @param s 输入的字符串常量引用
 * @return 处理后的字符串，已去除两端空白字符并转换为小写
 */
static std::string trim(const std::string &s)
{
    std::string out = s;
    out.erase(0, out.find_first_not_of(" \t\n\r"));
    out.erase(out.find_last_not_of(" \t\n\r") + 1);
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

/**
 * 创建一个具有指定列类型的数据库表
 * @param name 表名引用
 * @param cols 列定义向量引用
 */
void sqlDB::createTableWithTypes(std::string &name, std::vector<Column> &cols)
{
    std::string lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    if (tables.count(lname))
    {
        std::cout << "Table already exists. \n";
        return;
    }
    Table t;
    t.columns = cols;
    tables[lname] = t;
    tables[lname].saveToFile(lname);
    std::cout << "Table created with type. \n";
}

/**
 * @brief 向指定表插入一行数据
 *
 * 此方法会根据给定的表名、列和值，将新行插入到表中。
 * - 如果未指定列名 (cols 为空)，则要求 values 的数量与表列数完全一致。
 * - 如果指定了列名，则只更新这些列，未指定的列默认填充为 "NULL"。
 * - 插入完成后会调用 Table::saveToFile() 将表数据持久化。
 *
 * @param name 表名（不区分大小写，内部统一转换为小写）
 * @param values 插入的值列表，对应列的数据
 * @param cols 指定插入的列名（可选）。如果为空，则按照表列顺序插入。
 *
 * @note
 * - 若表不存在，会输出 "Table not found."
 * - 若列数与值数不匹配，会输出 "Column count mismatch."
 * - 若给定的列名在表中不存在，会输出 "Column not found: <列名>"
 *
 * @example
 * @code
 * sqlDB db;
 * std::vector<std::string> cols = {"id", "name"};
 * std::vector<std::string> vals = {"1", "Alice"};
 * db.insertInto("users", vals, cols);
 * @endcode
 */
void sqlDB::insertInto(std::string &name, const std::vector<std::string> &values, const std::vector<std::string> &cols)
{
    std::string lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    if (!tables.count(lname))
    {
        std::cout << "Table not found.\n";
        return;
    }
    Table &t = tables[lname];
    Row r;
    r.values.resize(t.columns.size(), "NULL");
    if (cols.empty())
    {
        if (t.columns.size() != values.size())
        {
            std::cout << "Column count mismatch.\n";
            return;
        }
        r.values = values; // copy values to row
    }
    else
    {
        if (cols.size() != values.size())
        {
            std::cout << "Column count mismatch.\n";
            return;
        }
        for (size_t i = 0; i < cols.size(); i++)
        {
            int idx = t.getColumnIndex(cols[i]);
            if (idx == -1)
            {
                std::cout << "Column not found:; " << cols[i] << std::endl;
                return;
            }
            r.values[idx] = values[i];
        }
        // 未指定的列保持默认值 "NULL"
    }
    t.rows.push_back(r);
    t.saveToFile(lname);
    std::cout << "Row inserted. " << std::endl;
}

/**
 * @brief 查询表中的数据并输出到标准输出
 *
 * 此方法实现了一个简易版的 SQL `SELECT` 功能，支持：
 * - 按条件筛选 (`WHERE`)
 * - 排序 (`ORDER BY`)
 * - 限制返回行数 (`LIMIT`)
 * - 升序 / 降序排列
 *
 * @param name 表名（不区分大小写，内部统一转换为小写）
 * @param whereCol 条件列名（可选，若为空则不使用筛选条件）
 * @param whereVal 条件值（与 whereCol 搭配使用，仅返回该列值等于 whereVal 的行）
 * @param orderBy 排序列名（可选，若为空则不排序）
 * @param desc 是否降序排列（true 表示降序，false 表示升序，默认升序）
 * @param limit 限制返回的最大行数（默认 -1 表示不限制）
 *
 * @note
 * - 若表不存在，会输出 `"Table not found."`
 * - 若条件列名或排序列名不存在，会输出 `"Column not found."`
 * - 返回结果直接打印到 `std::cout`，不存储在函数返回值中
 * - 排序时，比较是基于字符串字典序完成的，而非数值大小
 *
 * @example
 * @code
 * sqlDB db;
 * // 查询所有数据
 * db.selectAll("users");
 *
 * // 查询 age=30 的用户，并按 name 升序排列，最多返回 10 行
 * db.selectAll("users", "age", "30", "name", false, 10);
 *
 * // 查询并按 salary 降序排列
 * db.selectAll("employees", "", "", "salary", true);
 * @endcode
 */
void sqlDB::selectAll(const std::string &name, const std::string &whereCol, const std::string &whereVal, const std::string &orderBy, bool desc, int limit)
{
    std::string lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    if (!tables.count(lname))
    {
        std::cout << "Table not found.\n";
        return;
    }
    Table &t = tables[lname];
    for (const auto &col : t.columns)
        std::cout << col.name << "\t";
    std::cout << "\n";

    int colIdx = -1;
    if (!whereCol.empty())
    {
        colIdx = t.getColumnIndex(whereCol);
        if (colIdx == -1)
        {
            std::cout << "Column not found.\n";
            return;
        }
    }

    std::vector<std::size_t> rowIndices(t.rows.size());
    std::iota(rowIndices.begin(), rowIndices.end(), 0);
    int orderIdx = -1;
    if (!orderBy.empty())
    {
        orderIdx = t.getColumnIndex(orderBy);
        if (orderIdx == -1)
        {
            std::cout << "Column not found int ORDER BY. \n";
            return;
        }
    }
    std::sort(rowIndices.begin(), rowIndices.end(), [&](size_t a, size_t b)
              {
        if(desc)
            return t.rows[a].values[orderIdx] > t.rows[b].values[orderIdx];
        else
            return t.rows[a].values[orderIdx] < t.rows[b].values[orderIdx]; });

    int count = 0;
    for (size_t i : rowIndices)
    {
        const auto &row = t.rows[i];
        if (!whereCol.empty() && trim(row.values[colIdx]) != trim(whereVal))
            continue;
        for (auto &val : row.values)
            std::cout << val << "\t";
        std::cout << std::endl;
        if (limit > 0 && ++count >= limit)
            break;
    }
}

/**
 * @brief 更新表中满足条件的行
 *
 * 此方法会遍历指定表的所有行，对满足条件的记录，将目标列的值更新为新值。
 *
 * @param name 表名（不区分大小写，内部统一转换为小写）
 * @param targetCol 需要更新的列名
 * @param newVal 更新后的新值
 * @param whereCol 条件列名（仅更新该列值等于 whereVal 的行）
 * @param whereVal 条件值
 *
 * @note
 * - 若表不存在，则直接返回（无提示）
 * - 若目标列或条件列不存在，则输出 `"Column not found."`
 * - 更新完成后会调用 `Table::saveToFile()` 将修改后的表数据保存到文件
 * - 若没有行满足条件，则不会有任何更改，但仍会输出 `"Rows updated."`
 *
 * @example
 * @code
 * sqlDB db;
 * // 将 users 表中 age=30 的记录，其 name 更新为 "Alice"
 * db.update("users", "name", "Alice", "age", "30");
 * @endcode
 */
void sqlDB::update(const std::string &name, const std::string &targetCol, const std::string &newVal,
                   const std::string &whereCol, const std::string &whereVal)
{
    std::string lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    if(!tables.coungt(lname))
        return;
    Table& t = tables[lname];

    int targetIdx = t.getColumnIndex(targetCol);
    int whereIdx = t.getColumnIndex(whereCol);   
    if(targetIdx == -1 || whereIdx == -1)
    {
        std::cout<<"Column not found. \n";
        return;
    }
    for(auto& row: t.rows)
    {
        if(row.values[whereIdx] == whereVal)
        {
            row.values[targetIdx] = newVal;
        }
    }
    t.saveToFile(lname);
    std::cout<<"Rows updated. \n";
}

/**
 * @brief 删除表中满足条件的行
 *
 * 此方法会遍历指定表的所有行，并删除满足条件的记录：
 * - 条件列名为 @p whereCol
 * - 条件值为 @p whereVal
 *
 * @param name 表名（不区分大小写，内部统一转换为小写）
 * @param whereCol 条件列名
 * @param whereVal 条件值（仅删除该列值等于 whereVal 的行）
 *
 * @note
 * - 若表不存在，则直接返回（无提示）
 * - 若条件列不存在，则输出 `"Column not found."`
 * - 删除完成后会调用 `Table::saveToFile()` 将修改后的表数据保存到文件
 * - 删除逻辑基于 `std::remove_if` 实现
 *
 * @example
 * @code
 * sqlDB db;
 * // 删除 users 表中 age=30 的所有记录
 * db.deleteRows("users", "age", "30");
 * @endcode
 */

 void sqlDB::deleteRows(const std::string &name, const std::string &whereCol, const std::string &whereVal)
 {
      std::string lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    if(!tables.count(lname))
        return;
    Table& t = tables[lname];

    int whereIdx = t.getColumnIndex(whereCol);
    if(whereIdx == -1)
    {
        cout<<"Column not found. \n";
        return;
    }
    auto& rows = t.rows;
       rows.erase(std::remove_if(rows.begin(), rows.end(),
                              [&](const Row& r) {
                                  return r.values[whereIdx] == whereVal;
                              }),
               rows.end());
      t.saveToFile(lname);
    std::cout<<"Rows deleted. \n";
 }

/**
 * @brief 保存数据库中所有表的数据到文件
 *
 * 此方法会遍历当前数据库中的所有表，
 * 并依次调用每个表的 `Table::saveToFile()` 方法进行持久化存储。
 *
 * @note
 * - 文件名的生成方式依赖于 `Table::saveToFile()` 的实现，
 *   通常会与表名 (`it->first`) 关联。
 * - 若表数据较大，保存过程可能会耗时。
 * - 本方法不会返回成功/失败状态，所有结果直接由 `Table::saveToFile()` 负责。
 *
 * @example
 * @code
 * sqlDB db;
 * // 创建并插入数据 ...
 * db.saveAll(); // 将所有表写入文件
 * @endcode
 */
void sqlDB::saveAll()
{
     for (auto it = tables.begin(); it != tables.end(); ++it)
    {
        it->second.saveToFile(it->first);
    }
}

/**
 * @brief 从文件加载多个表到数据库
 *
 * 此方法会遍历给定的表名列表，逐个尝试从文件中加载表数据：
 * - 表名会统一转换为小写存储
 * - 每个表会调用 `Table::loadFromFile()` 读取持久化的数据
 * - 若加载的表包含有效列（`columns` 非空），则会被加入数据库
 *
 * @param tableNames 需要加载的表名列表
 *
 * @note
 * - 文件名的解析与加载逻辑依赖于 `Table::loadFromFile()` 的实现，
 *   通常与表名绑定。
 * - 若某个表文件不存在或内容为空（即 `columns` 为空），则不会加入数据库。
 * - 成功加载的表会输出 `"Loaded table: <表名>"`。
 *
 * @example
 * @code
 * sqlDB db;
 * std::vector<std::string> names = {"users", "orders"};
 * db.loadAll(names);  // 从文件加载 users 和 orders 表
 * @endcode
 */

void sqlDB::loadAll(const std::vector<std::string> &tableNames)
{
    for (const auto &name : tableNames)
    {
        std::string lname = name;
        std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
        Table t;
        t.loadFromFile(lname);
        if (!t.columns.empty())
        {
            tables[lname] = t;
            std::cout << "Loaded table: " << lname << "\n";
        }
    }
}

/**
 * @brief 删除指定的表
 *
 * 此方法会执行以下操作：
 * - 从内存中移除指定表（`tables.erase`）
 * - 删除磁盘上对应的表文件（通过 `getDbPath(name)` 获取路径，再调用 `std::remove` 删除）
 *
 * @param name 表名（不区分大小写，内部统一转换为小写）
 *
 * @note
 * - 若表存在于内存，则会从 `tables` 容器中移除
 * - 若对应的文件存在，删除成功后会输出 `"Table dropped and file deleted: <表名>"`
 * - 若文件不存在或删除失败，会输出 `"Table dropped (file not found or cannot delete): <表名>"`
 * - 使用 `std::remove` 删除文件，跨平台兼容
 *
 * @example
 * @code
 * sqlDB db;
 * db.dropTable("users");  // 删除名为 users 的表及其文件
 * @endcode
 */
void sqlDB::dropTable(const std::string &name)
{
    std::string lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    tables.erase(lname);
    // Remove file from disk
    std::string dropFile = getDbPath(name);
     if (std::remove(dropFile.c_str()) == 0)
    {
        std::cout << "Table dropped and file deleted: " << lname << "\n";
    }
    else
    {
        std::cout << "Table dropped (file not found or cannot delete): " << lname << "\n";
    }
}

/**
 * @brief 向指定表中添加新列
 *
 * 此方法会在目标表中追加一个新列，并为所有已有行增加对应的空值。
 * 新列的定义由参数 @p col 指定。
 *
 * @param tableName 表名（不区分大小写，内部统一转换为小写）
 * @param col 新增的列定义（包含列名和数据类型）
 *
 * @note
 * - 若表不存在，会输出 `"Table not found."` 并返回
 * - 新列会被追加到表的最后一列
 * - 已有行在新列上的默认值为空字符串 `""`
 * - 修改完成后会调用 `Table::saveToFile()` 将表持久化到磁盘
 * - 成功执行后，会输出 `"Column added: <列名>"`
 *
 * @example
 * @code
 * sqlDB db;
 * Column c{"email", DataType::STRING};
 * db.addColumn("users", c);
 * // users 表中会新增 email 列，已有行对应的值初始化为空
 * @endcode
 */
void sqlDB::addColumn(const std::string &tableName, const Column &col)
{
    std::string lname = tableName;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    if (!tables.count(lname))
    {
        std::cout << "Table not found.\n";
        return;
    }
    Table &t = tables[lname];
    t.columns.push_back(col);
    // Add empty value for new column to all existing rows
    for (auto &row : t.rows)
        row.values.push_back("");
    t.saveToFile(lname);
    std::cout << "Column added: " << col.name << "\n";
}

/**
 * @brief 从指定表中删除一个列
 *
 * 此方法会在目标表中移除指定的列，并同步删除所有行中该列对应的值。
 *
 * @param tableName 表名（不区分大小写，内部统一转换为小写）
 * @param colName   需要删除的列名
 *
 * @note
 * - 若表不存在，会输出 `"Table not found."` 并返回
 * - 若列不存在，会输出 `"Column not found."` 并返回
 * - 删除列后，所有行的数据会同步删除该列对应的值
 * - 修改完成后会调用 `Table::saveToFile()` 将表持久化到磁盘
 * - 成功执行后，会输出 `"Column dropped: <列名>"`
 *
 * @example
 * @code
 * sqlDB db;
 * // 删除 users 表中的 email 列
 * db.dropColumn("users", "email");
 * @endcode
 */
void sqlDB::dropColumn(const std::string &tableName, const std::string &colName)
{
    std::string lname = tableName;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    if (!tables.count(lname))
    {
        std::cout << "Table not found.\n";
        return;
    }
    Table &t = tables[lname];
    int idx = t.getColumnIndex(colName);
    if (idx == -1)
    {
        std::cout << "Column not found.\n";
        return;
    }
    t.columns.erase(t.columns.begin() + idx);
    for (auto &row : t.rows)
        row.values.erase(row.values.begin() + idx);
    t.saveToFile(lname);
    std::cout << "Column dropped: " << colName << "\n";
}

/**
 * @brief 对指定表的某一列执行聚合函数
 *
 * 支持的聚合函数包括：
 * - COUNT : 统计非 "NULL" 值的行数
 * - SUM   : 计算数值型列的总和
 * - AVG   : 计算数值型列的平均值（忽略 "NULL" 与空值）
 * - MIN   : 获取数值型列的最小值
 * - MAX   : 获取数值型列的最大值
 *
 * @param name 表名（不区分大小写，内部统一转换为小写）
 * @param func 聚合函数名称（COUNT, SUM, AVG, MIN, MAX，大小写敏感）
 * @param col 目标列名
 *
 * @note
 * - 若表不存在，会输出 `"Table not found."`
 * - 若列不存在，会输出 `"Column not found."`
 * - 对非数值型数据执行 SUM/AVG/MIN/MAX 时，无法转换的值会被忽略并打印异常信息
 * - 返回结果直接通过 `std::cout` 输出
 *
 * @example
 * @code
 * sqlDB db;
 * std::string col = "salary";
 * db.aggregate("employees", "SUM", col);   // 输出 SUM(salary)
 * db.aggregate("employees", "AVG", col);   // 输出 AVG(salary)
 * db.aggregate("employees", "COUNT", col); // 输出 COUNT(salary)
 * @endcode
 */
void sqlDB::aggregate(const std::string &name, const std::string &func, std::string &col)
{
    std::string lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    if (!tables.count(lname))
    {
        std::cout << "Table not found. \n";
        return;
    }
    Table &t = tables[lname];
    int idx = t.getColumnIndex(col);
    if (idx == -1)
    {
        std::cout << "Column not found. \n";
        return;
    }
    if (func == "COUNT")
    {
        int count = 0;
        for (auto &row : t.rows)
            if (row.values[idx] != "NULL")
                count++;
        std::cout << "COUNT(" << col << ") = " << count << std::endl;
    }
    else if (func == "SUM" || func == "AVG")
    {
        double sum = 0;
        int count = 0;
        for (const auto &row : t.rows)
        {
            if (!row.values[idx].empty() && row.values[idx] != "NULL")
            {
                try
                {
                    sum += std::stod(row.values[idx]);
                    ++count;
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
        if (func == "SUM")
            std::cout << "SUM(" << col << ") = " << sum << std::endl;
        else if (count > 0)
            std::cout << "AVG(" << col << ") = " << (sum / count) << std::endl;
        else
            std::cout << "AVG(" << col << ") = NULL\n";
    }
    else if (func == "MIN")
    {
        double minVal = std::numeric_limits<double>::max();
        bool found = false;
        for (auto &row : t.rows)
        {
            if (!row.values[idx].empty() && row.values[idx] != "NULL")
            {
                try
                {
                    double v = std::stod(row.values[idx]);
                    if (!found || v < minVal)
                    {
                        minVal = v;
                        found = true;
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
        if (found)
            std::cout << "MIN(" << col << ") = " << minVal << std::endl;
        else
            std::cout << "MIN(" << col << ") = NULL\n";
    }
    else if (func == "MAX")
    {
        double maxVal = std::numeric_limits<double>::lowest();
        bool found = false;
        for (const auto &row : t.rows)
        {
            if (row.values[idx] != "NULL" && !row.values[idx].empty())
            {
                try
                {
                    double v = std::stod(row.values[idx]);
                    if (!found || v > maxVal)
                    {
                        maxVal = v;
                        found = true;
                    }
                }
                catch (...)
                {
                }
            }
        }
        if (found)
            std::cout << "MAX(" << col << ") = " << maxVal << "\n";
        else
            std::cout << "MAX(" << col << ") = NULL\n";
    }
    else
    {
        std::cout << "Unknown aggregate function.\n";
    }
}

/**
 * @brief 获取数据库中所有表的名称列表
 * @return std::vector<std::string> 包含所有表名称的向量
 */
std::vector<std::string> sqlDB::listTables() const
{
    std::vector<std::string> names;
    for (const auto &pair : tables)
        names.push_back(pair.first);
    return names;
}