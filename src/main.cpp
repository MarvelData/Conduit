#include <string>

#include "StreamControl.hpp"
#include "Math.hpp"
#include "Date.hpp"
#include "PostInfo.hpp"
#include "Member.hpp"
#include "Database.hpp"
#include "Test.hpp"

using namespace std;

int main(int argc, char **argv)
{
    string regBookName = "../data/Conduit.md";
    StreamControl streamControl;
    if (argc > 1) {
        if (string(argv[1]) == "-silent")
            streamControl.Redirect();
        else
            regBookName = argv[1];
    }

    Database regBook(move(regBookName));
    while (regBook.TalkToUser());

    return 0;
}