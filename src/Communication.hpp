#pragma once

#include <iostream>
#include <string>

#include "Member.hpp"

class Database;

class Communication
{
    Database *database;

    void changeMemberData();

public:
    explicit Communication(Database *database);

    void FileProblems();

    bool TalkToUser();

    void PrintMember(Member &member, int counter = 0, bool moreInfo = false);

    static void Tabulator(const std::string &str, int threshold);

    static std::string ProceedLink(const std::string &link);

    static int AskForDecision();

    template <typename IterableContainer>
    static std::string ChooseElem(const IterableContainer &elems, bool exitPossible = false)
    {
        std::string elemName;
        bool correct = true;
        do {
            if (!correct)
                std::cout << std::endl << "No such element! Try again.";
            bool digits = true;
            std::cout << std::endl << "Input element name (u can also input according number)" << std::endl;
            if (exitPossible)
                std::cout << std::endl << "Or input -1 to return =)" << std::endl;
            std::cin >> elemName;
            if (exitPossible && elemName == "-1")
                return "";
            for (auto c : elemName)
                if (!isdigit(c)) {
                    digits = false;
                    break;
                }
            if (digits) {
                int counter = 0;
                for (auto &elem : elems)
                    if (stoi(elemName) == counter++) {
                        elemName = elem.first;
                        break;
                    }
            }
        } while (exitPossible && !(correct = bool(elems.count(elemName))));
        return elemName;
    }
};