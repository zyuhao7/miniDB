#include "table.h"

#ifdef _WIN32
#include <direct.h> // _mkdir
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h> // access
#define MKDIR(path) mkdir(path, 0755)
#endif

std::string getDbPath(const std::string &name)
{
    // 1. 获取用户目录
    const char *homeDir = nullptr;
#ifdef _WIN32
    homeDir = std::getenv("USERPROFILE");
#else
    homeDir = std::getenv("HOME");
#endif
    if (!homeDir)
        homeDir = "."; // fallback

    // 2. 拼接目录
    std::string dbDir = std::string(homeDir) + "/miniDB" + "/mydb_data";

    // 3. 创建目录（如果不存在）
#ifdef _WIN32
    if (MKDIR(dbDir.c_str()) != 0 && errno != EEXIST)
    {
        std::cerr << "mkdir failed: " << strerror(errno) << std::endl;
    }
#else
    if (access(dbDir.c_str(), F_OK) != 0)
    {
        if (MKDIR(dbDir.c_str()) != 0 && errno != EEXIST)
        {
            std::cerr << "mkdir failed: " << strerror(errno) << std::endl;
        }
    }
#endif

    // 4. 拼接文件路径
    return dbDir + "/" + name + ".table";
}
static std::string getTableFilePath(const std::string &name)
{
    std::string path = getDbPath(name);
    std::cout << "[DEBUG] Table file path: " << path << std::endl;
    return path;
}

int Table::getColumnIndex(const std::string &colName) const
{
    std::string name = colName;
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    for (size_t i = 0; i < columns.size(); i++)
    {
        std::string colName = columns[i].name;
        std::transform(colName.begin(), colName.end(), colName.begin(), ::tolower);
        if (colName == name)
            return i;
    }
    return -1;
}

void Table::saveToFile(const std::string &name)
{
    std::ofstream file(getTableFilePath(name));
    for (const auto &col : columns)
    {
        std::string typeStr;
        switch (col.type)
        {
        case DataType::INT:
            typeStr = "INT";
            break;
        case DataType::TEXT:
            typeStr = "TEXT";
            break;
        case DataType::FLOAT:
            typeStr = "FLOAT";
            break;
        case DataType::DOUBLE:
            typeStr = "DOUBLE";
            break;
        case DataType::DATE:
            typeStr = "DATE";
            break;
        case DataType::BOOL:
            typeStr = "BOOL";
            break;
        case DataType::VARCHAR:
            typeStr = "VARCHAR";
            break;
        }
        file << col.name << " " << typeStr << ",";
    }
    file << "\n";
    for (const auto &row : rows)
    {
        for (const auto &val : row.values)
            file << val << ",";
        file << "\n";
    }
    file.close();
}

void Table::loadFromFile(const std::string &name)
{
    std::ifstream file(getTableFilePath(name));
    if (!file)
        return;
    std::string line;
    if (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string col;
        while (std::getline(ss, col, ','))
        {
            if (!col.empty())
            {
                std::stringstream cs(col);
                std::string cname, ctype;
                cs >> cname >> ctype;
                DataType dtype;
                std::string ctypeUpper = ctype;
                std::transform(ctypeUpper.begin(), ctypeUpper.end(), ctypeUpper.begin(), ::toupper);
                if (ctypeUpper == "INT")
                    dtype = DataType::INT;
                else if (ctypeUpper == "TEXT")
                    dtype = DataType::TEXT;
                else if (ctypeUpper == "FLOAT")
                    dtype = DataType::FLOAT;
                else if (ctypeUpper == "DOUBLE")
                    dtype = DataType::DOUBLE;
                else if (ctypeUpper == "DATE")
                    dtype = DataType::DATE;
                else if (ctypeUpper == "BOOL")
                    dtype = DataType::BOOL;
                else if (ctypeUpper == "VARCHAR")
                    dtype = DataType::VARCHAR;
                else
                    continue;
                columns.push_back({cname, dtype});
            }
        }
    }
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string val;
        Row row;
        while (std::getline(ss, val, ','))
        {
            if (!val.empty())
                row.values.push_back(val);
        }
        if (!row.values.empty())
            rows.push_back(row);
    }
    file.close();
}