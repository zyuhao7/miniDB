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




struct Column
{
    std::string name;
    DataType type;
};

struct Row
{
    std::vector<std::string> values;
};

struct Table
{
    std::vector<Column> columns;
    std::vector<Row> rows;

    int getColumnIndex(const std::string &colName) const;
    void saveToFile(const std::string &filename);
    void loadFromFile(const std::string &filename);
};
