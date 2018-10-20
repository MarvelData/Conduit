#pragma once

#include <string>

#include "Database.hpp"

class Test
{
    Database database;

public:
    Test(size_t membersAmount, size_t postsAmount);

    static std::string GetRandomString(int n);
};

