#include "Test.hpp"

#include <iostream>

#include "Member.hpp"

using namespace std;

Test::Test(size_t membersAmount = 1000, size_t postsAmount = 1000) : database("../TestData/Test.data")
{
    for (size_t i = 0; i < membersAmount; i++) {
        string name = GetRandomString(10);
        string savedName = name;
        database.AddMember(move(name), move(GetRandomString(15)), move(GetRandomString(15)), 2, "2018.10.10");
        for (size_t j = 0; j < postsAmount; j++) {
            string link = GetRandomString(20);
            database.AddPost(savedName, to_string(j + 1000) + ".10.10", link);
        }
    }
    database.WriteDatabaseToFiles();
    while (database.TalkToUser());
}

string Test::GetRandomString(int n)
{
    string result;
    for (size_t i = 0; i < n; i++)
        result += char(rand() % ('z' - 'a' + 1) + 'a');
    return result;
}

void Database::AddMember(string shortName, string role, string rubric, int frequency, string startDate)
{
    if (data.count(shortName))
        cout << endl << "This short name is already used :(" << endl;
    else {
        Member member(move(shortName), "", move(role), move(rubric), frequency, move(startDate), GetPath());
        member.ForceDeepInfoUpdate();
        data[member.GetShortName()] = move(member);
    }
}

void Database::AddPost(const string &shortName, const string &date, string &link)
{
    data.at(shortName).AddPost(date, link);
}