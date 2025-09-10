#include "db.h"
#include "parser.h"
#include <filesystem>
#include <vector>
#include <string>

int main()
{
    sqlDB db;
    std::filesystem::path dbDir = std::filesystem::path(std::getenv("HOME")) /  "miniDB/mydb_data";
    std::vector<std::string> tableNames;
    if (std::filesystem::exists(dbDir))
    {
        for (const auto &entry : std::filesystem::directory_iterator(dbDir))
        {
            std::string fname = entry.path().filename().string();
            if (fname.size() > 6 && fname.substr(fname.size() - 6) == ".table")
            {
                tableNames.push_back(fname.substr(0, fname.size() - 6));
            }
        }
    }
    db.loadAll(tableNames);
    runSQLConsole(db);
    db.saveAll();
    return 0;
}