#pragma once

#include <string>

struct PostInfo
{
    std::string ShortName;
    std::string Date;
    std::string Link;
    int Index;
    bool Correct;

    PostInfo();
    PostInfo(const std::string &shortName, const std::string &date, const std::string &link, int index);
};