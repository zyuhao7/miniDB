#include "parser.h"
#include "types.h"
#include <iostream>
#include <sstream>
#include <algorithm>

/**
 * @brief 运行一个交互式 SQL 控制台
 *
 * 该函数会进入一个循环，从标准输入中读取用户输入的 SQL 命令，
 * 然后解析命令并调用对应的 @ref sqlDB 成员函数来执行。
 * 支持的命令包括：
 * - CREATE TABLE
 * - INSERT INTO
 * - SELECT
 * - UPDATE
 * - DELETE
 * - DROP TABLE
 * - SHOW TABLES
 * - ALTER TABLE (ADD / DROP 列)
 * - 退出：输入 `exit`
 *
 * @param db 数据库对象的引用，所有操作都会作用在该数据库上。
 */
/**
 * @brief 运行一个交互式 SQL 控制台
 *
 * 该函数会进入一个循环，从标准输入中读取用户输入的 SQL 命令，
 * 然后解析命令并调用对应的 @ref sqlDB 成员函数来执行。
 * 支持的命令包括：
 * - CREATE TABLE
 * - INSERT INTO
 * - SELECT
 * - UPDATE
 * - DELETE
 * - DROP TABLE
 * - SHOW TABLES
 * - ALTER TABLE (ADD / DROP 列)
 * - 退出：输入 `exit`
 *
 * @param db 数据库对象的引用，所有操作都会作用在该数据库上。
 */
void runSQLConsole(sqlDB &db)
{
    std::string line;
    std::cout << "Enter SQL Commands (type 'exit' to quit): \n";

    while (true)
    {
        std::cout << ">> ";
        std::getline(std::cin, line); ///< 从标准输入读取一行命令
        if (line == "exit")
            break;

        std::stringstream ss(line); ///< 用 stringstream 解析命令
        std::string cmd;
        ss >> cmd;

        // 将命令转为大写（方便大小写无关的 SQL 解析）
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        /** ========== CREATE TABLE 处理 ========== */
        if (cmd == "CREATE")
        {
            std::string tbl, name;
            ss >> tbl >> name;
            if (tbl != "TABLE")
            {
                std::cout << "Invalid CREATE syntax. Use: CREATE TABLE <table_name> (<col1> <type1>, ...)\n";
                continue;
            }

            // 移除可能的多余字符
            if (!name.empty() && name.back() == '(')
                name.pop_back();
            name.erase(std::remove_if(name.begin(), name.end(), ::isspace), name.end());

            // 提取列定义
            std::string res;
            std::getline(ss, res, '(');
            std::getline(ss, res, ')');
            std::stringstream def(res);
            std::string col;
            std::vector<Column> cols;

            // 解析每个列定义（列名 + 类型）
            while (std::getline(def, col, ','))
            {
                std::stringstream cs(col);
                std::string cname, ctype;
                cs >> cname >> ctype;
                if (ctype.empty() && !(cs >> ctype))
                {
                    std::cerr << "Invalid column definition: " << col << "\n";
                    continue;
                }

                // 去掉类型中的括号，例如 varchar(20) -> varchar
                size_t paren = ctype.find('(');
                if (paren != std::string::npos)
                    ctype = ctype.substr(0, paren);
                ctype.erase(std::remove_if(ctype.begin(), ctype.end(), ::isspace), ctype.end());

                cols.push_back({cname, parseType(ctype)});
            }
            db.createTableWithTypes(name, cols);
        }
        /** ========== INSERT INTO 处理 ========== */
        else if (cmd == "INSERT")
        {
            std::string into, table, colPart, values;
            ss >> into >> table;
            std::vector<std::string> columns;

            // 如果有指定列名 (INSERT INTO t (col1, col2) VALUES ...)
            char c = ss.peek();
            if (c == '(')
            {
                ss.get();
                std::getline(ss, colPart, ')');
                std::stringstream cs(colPart);
                std::string col;
                while (std::getline(cs, col, ','))
                {
                    col.erase(std::remove_if(col.begin(), col.end(), ::isspace), col.end());
                    columns.push_back(col);
                }
                ss >> values;
            }
            else
            {
                ss >> values;
            }

            // 提取 VALUES
            std::getline(ss, values, '(');
            std::getline(ss, values, ')');
            std::stringstream vs(values);
            std::string val;
            std::vector<std::string> vals;
            while (std::getline(vs, val, ','))
            {
                // 去掉多余引号和空格
                val.erase(std::remove(val.begin(), val.end(), '\''), val.end());
                val.erase(0, val.find_first_not_of(" \t\n\r"));
                val.erase(val.find_last_not_of(" \t\n\r") + 1);
                vals.push_back(val);
            }
            db.insertInto(table, vals, columns);
        }
        /** ========== SELECT 处理 ========== */
        else if (cmd == "SELECT")
        {
            std::string star, from, table;
            ss >> star >> from >> table;

            // 判断是否为聚合函数 (SUM/AVG/MIN/MAX/COUNT)
            if (star.find("(") != std::string::npos && star.find(")") != std::string::npos)
            {
                size_t l = star.find("(");
                size_t r = star.find(")");
                std::string func = star.substr(0, l);
                std::string col = star.substr(l + 1, r - l + 1);

                std::string tbl;
                if (from == "FROM" && !table.empty())
                {
                    tbl = table;
                    while (!tbl.empty() && (tbl.back() == ';' || std::isspace(tbl.back())))
                        tbl.pop_back();
                }
                db.aggregate(tbl, func, col);
                continue;
            }

            // 去掉末尾多余符号
            while (!table.empty() && (table.back() == ';' || std::isspace(table.back())))
                table.pop_back();

            std::string whereCol, whereVal, orderBy;
            bool desc = false;
            int limit = -1;

            // 解析 WHERE 子句
            std::string where, col, eq, val;
            std::streampos pos = ss.tellg();
            if (ss >> where >> col >> eq >> val)
            {
                std::transform(where.begin(), where.end(), where.begin(), ::toupper);
                if (where == "WHERE")
                {
                    val.erase(std::remove(val.begin(), val.end(), '\''), val.end());
                    // 去掉末尾分号
                    if (!val.empty() && val.back() == ';')
                        val.pop_back();
                    whereCol = col;
                    whereVal = val;
                }
                else
                {
                    ss.clear();
                    ss.seekg(pos);
                }
            }

            // 解析 ORDER BY 子句
            std::string order, by, orderCol, orderDir;
            if (ss >> order >> by >> orderCol)
            {
                std::transform(order.begin(), order.end(), order.begin(), ::toupper);
                std::transform(by.begin(), by.end(), by.begin(), ::toupper);
                if (ss >> orderDir)
                {
                    std::transform(orderDir.begin(), orderDir.end(), orderDir.begin(), ::toupper);
                    desc = (orderDir == "DESC");
                }
                orderBy = orderCol;
            }

            // 解析 LIMIT
            std::string limitStr;
            if (ss >> limitStr)
            {
                std::transform(limitStr.begin(), limitStr.end(), limitStr.begin(), ::toupper);
                if (limitStr == "LIMIT")
                {
                    ss >> limit;
                }
            }

            db.selectAll(table, whereCol, whereVal, orderBy, desc, limit);
        }
        /** ========== UPDATE 处理 ========== */
        else if (cmd == "UPDATE")
        {
            std::string table, set, col, eq, val, where, wcol, weq, wval;
            ss >> table >> set >> col >> eq >> val >> where >> wcol >> weq >> wval;

            while (!table.empty() && (table.back() == ';' || std::isspace(table.back())))
                table.pop_back();

            val.erase(std::remove(val.begin(), val.end(), '\''), val.end());
            wval.erase(std::remove(wval.begin(), wval.end(), '\''), wval.end());
            db.update(table, col, val, wcol, wval);
        }
        /** ========== DELETE 处理 ========== */
        else if (cmd == "DELETE")
        {
            std::string from, table, where, col, eq, val;
            ss >> from >> table >> where >> col >> eq >> val;

            while (!table.empty() && (table.back() == ';' || std::isspace(table.back())))
                table.pop_back();

            val.erase(std::remove(val.begin(), val.end(), '\''), val.end());
            db.deleteRows(table, col, val);
        }
        /** ========== DROP TABLE 处理 ========== */
        else if (cmd == "DROP" || cmd == "DROP;")
        {
            std::string tbl, name;
            ss >> tbl >> name;
            while (!name.empty() && (name.back() == ';' || std::isspace(name.back())))
                name.pop_back();

            db.dropTable(name);
        }
        /** ========== SHOW TABLES 处理 ========== */
        else if (cmd == "SHOW" || cmd == "SHOW;")
        {
            std::string what;
            ss >> what;
            std::transform(what.begin(), what.end(), what.begin(), ::toupper);
            if (what == "TABLES" || what == "TABLES;")
            {
                auto tables = db.listTables();
                std::cout << "Tables:\n";
                for (const auto &t : tables)
                    std::cout << t << std::endl;
            }
            else
            {
                std::cout << "Invalid SHOW command.\n";
            }
        }
        /** ========== ALTER TABLE 处理 ========== */
        else if (cmd == "ALTER" || cmd == "ALTER;")
        {
            std::string tbl, name, op, col, ctype;
            ss >> tbl >> name >> op >> col >> ctype;

            while (!name.empty() && (name.back() == ';' || std::isspace(name.back())))
                name.pop_back();
            while (!col.empty() && (col.back() == ';' || std::isspace(col.back())))
                col.pop_back();

            if (op == "ADD")
            {
                // 去掉类型括号，例如 varchar(20) -> varchar
                size_t paren = ctype.find('(');
                if (paren != std::string::npos)
                    ctype = ctype.substr(0, paren);
                while (!ctype.empty() && (ctype.back() == ';' || ::isspace(ctype.back())))
                    ctype.pop_back();
                while (!ctype.empty() && ::isspace(ctype.front()))
                    ctype.erase(ctype.begin());

                db.addColumn(name, {col, parseType(ctype)});
            }
            else if (op == "DROP")
            {
                db.dropColumn(name, col);
            }
            else
            {
                std::cout << "Invalid ALTER TABLE command.\n";
            }
        }
        else
        {
            std::cout << "Invalid SQL command.\n";
        }
    }
}
