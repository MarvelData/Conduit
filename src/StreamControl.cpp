#include "StreamControl.hpp"

#include <iostream>

using namespace std;

void StreamControl::Redirect(const string &name)
{
    out.open(name);
    cout.rdbuf(out.rdbuf());
}

StreamControl::~StreamControl() { out.close(); }