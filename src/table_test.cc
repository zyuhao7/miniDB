#include "table.h"
#include <iostream>
#include <cassert>

int main()
{
    // 创建一个 Table 对象
    Table table;

    // 定义列
    table.columns = {
        {"id", DataType::INT},
        {"name", DataType::TEXT},
        {"age", DataType::INT}};

    // 定义行
    table.rows = {
        {{"1", "Alice", "30"}},
        {{"2", "Bob", "25"}},
        {{"3", "Charlie", "35"}}};

    // 保存到文件
    std::string tableName = "test_table";
    table.saveToFile(tableName);

    // 创建另一个 Table 对象并从文件加载
    Table loadedTable;
    loadedTable.loadFromFile(tableName);

    // 验证列是否正确
    assert(loadedTable.columns.size() == table.columns.size());
    for (size_t i = 0; i < table.columns.size(); ++i)
    {
        assert(loadedTable.columns[i].name == table.columns[i].name);
        assert(loadedTable.columns[i].type == table.columns[i].type);
    }

    // 验证行是否正确
    assert(loadedTable.rows.size() == table.rows.size());
    for (size_t i = 0; i < table.rows.size(); ++i)
    {
        assert(loadedTable.rows[i].values == table.rows[i].values);
    }

    // 输出测试结果
    std::cout << "All tests passed!" << std::endl;

    return 0;
}