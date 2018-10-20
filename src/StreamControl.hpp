#pragma once

#include <fstream>
#include <string>

class StreamControl
{
    std::ofstream out;

public:
    void Redirect(const std::string &name = "TempRegBookOut.txt");

    ~StreamControl();
};
