#pragma once
#include <string>
#include "db.h"

/**
 * @brief 启动交互式 SQL 控制台
 *
 * 该函数会进入一个交互式命令行界面，用户可以输入 SQL 语句
 * （如 CREATE、INSERT、SELECT 等），由内部的 sqlDB 对象解析并执行。
 *
 * @param db 引用传入的 sqlDB 数据库实例，用于执行用户输入的 SQL 语句。
 *
 * ### 使用示例
 * @code
 * sqlDB db;
 * db.createTable("students", {"id", "name", "age"});
 * runSQLConsole(db);  // 启动控制台，等待用户输入 SQL 语句
 * @endcode
 *
 * @note
 * - 控制台一般会以循环方式持续运行，直到用户输入退出指令（如 EXIT/QUIT）
 *
 * @date
 *  2025/9/10
 *
 * @author
 *  moyuh
 */
void runSQLConsole(sqlDB &db);