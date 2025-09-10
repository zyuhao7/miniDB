#include <iostream>
#include "db.h"

int main()
{
    sqlDB db;

    // 1. 创建表
    std::string userTable = "users";
    std::vector<Column> cols = {
        {"id", DataType::INT},
        {"name", DataType::VARCHAR},
        {"age", DataType::INT},
        {"salary", DataType::INT}};

    db.createTableWithTypes(userTable, cols);

    // 2. 插入数据
    db.insertInto(userTable, {"1", "Alice", "23", "5000"}, {});
    db.insertInto(userTable, {"2", "Bob", "30", "8000"}, {});
    db.insertInto(userTable, {"3", "Cathy", "27", "7000"}, {});
    db.insertInto(userTable, {"4", "David", "30", "6500"}, {});

    std::cout << "=== 初始数据 ===" << std::endl;
    db.selectAll(userTable);

    // 3. 带 where 查询
    std::cout << "\n=== 查询 age=30 的用户 ===" << std::endl;
    db.selectAll(userTable, "age", "30");

    // 4. 排序 + limit
    std::cout << "\n=== 按薪资排序, 取前2名 ===" << std::endl;
    db.selectAll(userTable, "", "", "salary", true, 2);

    // 5. 更新数据
    std::cout << "\n=== 将 Bob 的薪资改为 9000 ===" << std::endl;
    db.update(userTable, "salary", "9000", "name", "Bob");
    db.selectAll(userTable);

    // 6. 删除数据
    std::cout << "\n=== 删除 age=27 的用户 ===" << std::endl;
    db.deleteRows(userTable, "age", "27");
    db.selectAll(userTable);

    // 7. 聚合函数
    std::string salaryCol = "salary";
    std::cout << "\n=== 聚合函数测试 ===" << std::endl;
    db.aggregate(userTable, "sum", salaryCol);
    db.aggregate(userTable, "avg", salaryCol);
    db.aggregate(userTable, "min", salaryCol);
    db.aggregate(userTable, "max", salaryCol);
    db.aggregate(userTable, "count", salaryCol);

    // 8. 添加列
    std::cout << "\n=== 添加列 address ===" << std::endl;
    db.addColumn(userTable, {"address", DataType::TEXT});
    db.selectAll(userTable);

    // 9. 删除列
    std::cout << "\n=== 删除列 salary ===" << std::endl;
    db.dropColumn(userTable, "salary");
    db.selectAll(userTable);

    // 10. 保存与加载
    std::cout << "\n=== 保存表 ===" << std::endl;
    db.saveAll();

    std::cout << "\n=== 加载表 ===" << std::endl;
    db.loadAll({userTable});
    db.selectAll(userTable);

    // 11. 删除表
    std::cout << "\n=== 删除表 users ===" << std::endl;
    db.dropTable(userTable);

    std::cout << "\n=== 当前数据库所有表 ===" << std::endl;
    auto tables = db.listTables();
    for (auto &t : tables)
        std::cout << t << std::endl;

    return 0;
}
