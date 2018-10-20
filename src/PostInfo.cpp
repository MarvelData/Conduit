#include "PostInfo.hpp"

using namespace std;

#include <iostream>

PostInfo::PostInfo() : Index(-1), Correct(false) {}

PostInfo::PostInfo(const string &shortName, const string &date, const string &link, int index) :
        ShortName(shortName), Date(date), Link(link), Index(index), Correct(true)
{
    if (ShortName.find(' ') != -1 || Date.find(' ') != -1 || Link.find(' ') != -1) {
        cout << endl << "Short names, roles and rubrics can't contain spaces, srry :D" << endl;
        throw exception();
    }
}