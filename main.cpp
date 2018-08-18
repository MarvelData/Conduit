#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <random>
#include <set>
#include <stdexcept>
#include <string>

using namespace std;

string GetRandomString(int n)
{
    string result;
    for (size_t i = 0; i < n; i++)
        result += rand() % ('z' - 'a' + 1) + 'a';
    return result;
}

int GreatestCommonDivisor(int a, int b) { return b ? GreatestCommonDivisor(b, a % b) : a; }

int LeastCommonMultiplier(int a, int b) { return a / GreatestCommonDivisor(a, b) * b; }

class Date
{
    string date;
    int day, month, year;
    size_t daySymbols = 2, monthSymbols = 2, yearSymbols = 4, dotsAmount = 2;

public:
    explicit Date(const string &date) : date(date)
    {
        if (CheckDate(date)) {
            year = stoi(date.substr(0, yearSymbols));
            month = stoi(date.substr(yearSymbols + 1, monthSymbols));
            day = stoi(date.substr(yearSymbols + monthSymbols + dotsAmount, daySymbols));
        } else {
            day = month = year = -1;
            DateProblems();
        }
    }

    static void DateProblems()
    {
        cout << endl << "Something is wrong with your date, it should look like YYYY.MM.DD :)" << endl;
    }

    static bool CheckDate(const string &date)
    {
        if (date.length() != 10)
            return false;
        if (date[4] != '.' || date[7] != '.')
            return false;
        for (auto c : date)
            if (c != '.' && !isdigit(c))
                return false;
        if (stoi(date.substr(8, 2)) < 1 || stoi(date.substr(8, 2)) > 31)
            return false;
        if (stoi(date.substr(5, 2)) < 1 || stoi(date.substr(5, 2)) > 12)
            return false;

        return true;
    }

    friend ostream& operator<<(ostream& os, const Date& dt);
};

ostream& operator<<(ostream& os, const Date& date)
{
    os << date.date;
    return os;
}

class Member
{
    string shortName;
    string role;
    string rubric;
    int frequency;
    Date start;
    int postsAmount;
    map<string, vector<string>> posts;

public:
    Member() : postsAmount(0), start("0000.01.01") {}

    Member(string&& shortName, string &&role, string &&rubric, int frequency, string &&start) :
            shortName(shortName), role(role), rubric(rubric), frequency(frequency), start(start), postsAmount(0)
    {
        if (shortName.find(' ') != -1 || role.find(' ') != -1 || rubric.find(' ') != -1) {
            cout << endl << "Short names, roles and rubrics can't contain spaces, srry :D" << endl;
            throw exception();
        }
    }

    bool AddPost(const string &date, const string &link)
    {
        if (Date::CheckDate(date)) {
            if (posts.count(date))
                posts[date].emplace_back(link);
            else
                posts[date] = vector<string>(1, link);
            postsAmount++;
            return true;
        } else {
            Date::DateProblems();
            return false;
        }
    }

    void DeletePost(const string &date, const string &link)
    {
        if (Date::CheckDate(date)) {
            if (posts.count(date)) {
                if (find(posts[date].begin(), posts[date].end(), link) == posts[date].end()) {
                    cout << endl << "There is no such post ..." << endl;
                    return;
                }
                if (posts[date].size() > 1)
                    posts[date].erase(find(posts[date].begin(), posts[date].end(), link));
                else
                    posts.erase(date);
                postsAmount--;
            } else {
                cout << endl << "There is no such post ..." << endl;
                return;
            }
        } else
            Date::DateProblems();
    }

    vector<string> GetPosts(const string &date) const
    {
        if (Date::CheckDate(date)) {
            return posts.at(date);
        }
        Date::DateProblems();
        return vector<string>();
    }

    void PrintPosts(ostream &os) const
    {
        for (auto &post : posts) {
            os << post.first << ' ' << post.second.size() << ' ';
            for (auto &link : post.second)
                os << link << ' ';
            os << endl;
        }
    }

    void PrintInfo() const
    {
        cout << endl << "Role: " << role << endl << "Rubric: " << rubric
             << endl << "Frequency: " << frequency << endl << "Start date: " << start << endl;
    }

    void Rename(const string &newShortName) { shortName = newShortName; }

    void ChangeRole(const string &newRole) { role = newRole; }

    void ChangeRubric(const string &newRubric) { rubric = newRubric; }

    void ChangeFrequency(int newFrequency) { frequency = newFrequency; }

    string GetShortName() const { return shortName; }

    string GetRole() const { return role; }

    string GetRubric() const { return rubric; }

    int GetFrequency() const { return frequency; }

    Date GetDate() const { return start; }

    int GetPostsAmount() const {
        int counter = 0;
        for (auto &post : posts)
            for (auto &link : post.second)
                counter++;
        if (counter != postsAmount)
            cout << endl << "DATABASE HAS PROBLEMS!!! AT LEAST WITH POSTS AMOUNTS!" << endl;
        return postsAmount;
    }

    size_t GetPostsDatesAmount() const { return posts.size(); }
};

class Database
{
    struct PostInfo
    {
        string ShortName;
        string Date;
        string Link;
        bool Correct;

        PostInfo() : Correct(false) {}
        PostInfo(string&& shortName, string &&date, string &&link) :
                ShortName(shortName), Date(date), Link(link), Correct(true)
        {
            if (ShortName.find(' ') != -1 || Date.find(' ') != -1 || Link.find(' ') != -1) {
                cout << endl << "Short names, roles and rubrics can't contain spaces, srry :D" << endl;
                throw exception();
            }
        }
    };

    string fileName;
    map<string, Member> data;

    void addMember()
    {
        printMembers();
        string shortName, role, rubric, startDate;
        int frequency;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input member short name: ";
        cin >> shortName;
        if (shortName.empty()) {
            cout << endl << "You can't add empty short name :(" << endl;
            return;
        }
        cout << endl << "Input member role: ";
        cin >> role;
        cout << endl << "Input member rubric: ";
        cin >> rubric;
        cout << endl << "Input frequency: ";
        cin >> frequency;
        cout << endl << "Input start date: ";
        cin >> startDate;
        if (data.count(shortName))
            cout << endl << "This short name is already used :(" << endl;
        else {
            Member member(move(shortName), move(role), move(rubric), frequency, move(startDate));
            data[member.GetShortName()] = member;
            cout << endl << "New member is successfully added!" << endl;
        }
        WriteDatabaseToFile();
    }

    void printMembers()
    {
        cout << endl << "Current members list:" << endl;
        int counter = 0;
        for (auto &elem : data)
            cout << counter++ << ". " << elem.first << endl;
    }

    string collectMemberName()
    {
        string shortName;
        printMembers();
        cout << endl << "Input member (u can also input according number): ";
        cin >> shortName;
        for (auto c : shortName)
            if (!isdigit(c)) {
                cout << endl << "Here are his/her posts: " << endl;
                data[shortName].PrintPosts(cout);
                return shortName;
            }
        int counter = 0;
        for (auto &elem : data)
            if (stoi(shortName) == counter++) {
                shortName = elem.first;
                break;
            }
        cout << endl << "Here are his/her posts: " << endl;
        data.at(shortName).PrintPosts(cout);
        return shortName;
    }

    PostInfo collectPostInfo()
    {
        string shortName, date, link;
        shortName = collectMemberName();
        cout << endl << "Input date in format YYYY.MM.DD (if u want to use current date, just input 0): ";
        cin >> date;
        if (date == "0") {
            auto time = chrono::system_clock::to_time_t(chrono::system_clock::now());
            date = to_string(localtime(&time)->tm_year + 1900) + '.';
            if (localtime(&time)->tm_mon + 1 < 10)
                date += '0';
            date += to_string(localtime(&time)->tm_mon + 1) + '.';
            if (localtime(&time)->tm_mday < 10)
                date += '0';
            date += to_string(localtime(&time)->tm_mday);
        }
        cout << endl << "Input link: ";
        cin >> link;
        if (!data.count(shortName)) {
            cout << endl << "There is no such member!" << endl;
            return PostInfo();
        }
        return PostInfo(move(shortName), move(date), move(link));
    }

    void addPost()
    {
        PostInfo postInfo = collectPostInfo();
        if (postInfo.Correct)
            data[postInfo.ShortName].AddPost(postInfo.Date, postInfo.Link);
        WriteDatabaseToFile();
    }

    void learnAboutMembers()
    {
        printMembers();
        cout << endl << "Would u like to learn something about exact member?" << endl;
        if (AskForDecision()) {
            string shortName = collectMemberName();
            data[shortName].PrintInfo();
        }
    }

    void learnAboutRoles()
    {
        set<string> roles;
        for (auto &elem : data)
            roles.emplace(elem.second.GetRole());
        cout << endl << "Current roles list:" << endl;
        for (auto &role : roles)
            cout << role << endl;
    }

    void learnAboutRubrics()
    {
        map<string, pair<int, vector<int>>> rubrics;
        for (auto &elem : data) {
            if (rubrics.count(elem.second.GetRubric()))
                rubrics[elem.second.GetRubric()].first++;
            else
                rubrics[elem.second.GetRubric()] = pair<int, vector<int>>(1, vector<int>());
            rubrics[elem.second.GetRubric()].second.emplace_back(elem.second.GetFrequency());
        }
        vector<double> statistics;
        for (auto &rubric : rubrics) {
            int leastCommonMultiplier = 1, sum = 0;
            for (auto frequency : rubric.second.second)
                leastCommonMultiplier = LeastCommonMultiplier(leastCommonMultiplier, frequency);
            for (auto frequency : rubric.second.second)
                sum += leastCommonMultiplier / frequency;
            statistics.emplace_back(double(leastCommonMultiplier) / sum);
        }
        cout << endl << "Current rubrics list:" << endl;
        int counter = 0;
        for (auto &rubric : rubrics) {
            cout << rubric.first << ": Editors amount: " << rubric.second.first
                 << " Overall frequency: " << statistics[counter] << " (" << ceil(statistics[counter]) << ')' << endl;
            counter++;
        }
    }

    void deletePost()
    {
        PostInfo postInfo = collectPostInfo();
        if (postInfo.Correct)
            data[postInfo.ShortName].DeletePost(postInfo.Date, postInfo.Link);
        WriteDatabaseToFile();
    }

    void deleteMember()
    {
        string shortName = collectMemberName();
        if (data.count(shortName))
            data.erase(shortName);
        else
            cout << endl << "There is no such member already!" << endl;
        WriteDatabaseToFile();
    }

    void changeMemberShortName()
    {
        string oldShortName = collectMemberName();
        string newShortName;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input new short name for this member: ";
        cin >> newShortName;
        if (newShortName.empty()) {
            cout << endl << "You can't add empty short name :(" << endl;
            return;
        }
        if (newShortName == oldShortName) {
            cout << endl << "U made no changes :D" << endl;
            return;
        }
        if (data.count(newShortName)) {
            cout << endl << "This new short name is already used" << endl;
            return;
        }
        data[oldShortName].Rename(newShortName);
        data[newShortName] = move(data[oldShortName]);
        data.erase(oldShortName);
        WriteDatabaseToFile();
    }

    void changeMemberRole()
    {
        string shortName = collectMemberName();
        data[shortName].PrintInfo();
        string newRole;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input new role for this member: ";
        cin >> newRole;
        data[shortName].ChangeRole(newRole);
        WriteDatabaseToFile();
    }

    void changeMemberRubric()
    {
        string shortName = collectMemberName();
        data[shortName].PrintInfo();
        string newRubric;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input new rubric for this member: ";
        cin >> newRubric;
        data[shortName].ChangeRubric(newRubric);
        WriteDatabaseToFile();
    }

    void changeMemberFrequency()
    {
        string shortName = collectMemberName();
        data[shortName].PrintInfo();
        int newFrequency;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input new frequency for this member: ";
        cin >> newFrequency;
        data[shortName].ChangeFrequency(newFrequency);
        WriteDatabaseToFile();
    }

    void changeMemberData()
    {
        printMembers();
        cout << endl << "What would u like to change?" << endl;
        cout << "0. I would like to change member short name" << endl;
        cout << "1. I would like to change member role" << endl;
        cout << "2. I would like to change member rubric" << endl;
        cout << "3. I would like to change member frequency" << endl;
        cout << endl << "Input appropriate number =)" << endl;

        int decision;
        cin >> decision;
        switch (decision) {
            case 0:
                changeMemberShortName();
                break;
            case 1:
                changeMemberRole();
                break;
            case 2:
                changeMemberRubric();
                break;
            case 3:
                changeMemberFrequency();
                break;
            default:
                cout << endl << "You made some mistake :(" << endl;
                break;
        }
    }

public:
    explicit Database(string &&fileName) : fileName(fileName)
    {
        ifstream file(fileName);

        if (!file.is_open()) {
            FileProblems();
            ofstream newFile(fileName);
            newFile.close();
            return;
        }

        string buf;
        int membersAmount;
        file >> buf >> buf;
        file >> membersAmount;
        for (int i = 0; i < membersAmount; i++) {
            string shortName, role, rubric, startDate;
            int frequency, postsDatesAmount;
            file >> shortName >> role >> rubric;
            file >> frequency;
            file >> startDate;
            Member member(move(shortName), move(role), move(rubric), frequency, move(startDate));
            file >> buf >> buf >> buf >> buf >> buf >> buf >> buf;
            file >> postsDatesAmount;
            for (int j = 0; j < postsDatesAmount; j++) {
                string date, link;
                int amount;
                file >> date;
                file >> amount;
                for (int k = 0; k < amount; k++) {
                    file >> link;
                    member.AddPost(date, link);
                }
            }
            data[member.GetShortName()] = move(member);
        }

        file.close();
        for (auto &elem : data)
            elem.second.GetPostsAmount();
    }

    void WriteDatabaseToFile()
    {
        for (auto &elem : data)
            elem.second.GetPostsAmount();
        ofstream file(fileName);

        file << "Members amount: " << data.size() << endl << endl;
        for (auto &elem : data) {
            file << elem.first << '\t' << elem.second.GetRole() << ' ' << elem.second.GetRubric()
                 << ' ' << elem.second.GetFrequency() << ' ' << elem.second.GetDate() << endl;
            file << "Total posts amount: " << elem.second.GetPostsAmount() << '\t'
                 << "Posts dates amount: " << elem.second.GetPostsDatesAmount() << endl;
            elem.second.PrintPosts(file);
            file << endl;
        }

        file.close();
    }

    void AddMember(string shortName, string role, string rubric, int frequency, string startDate)
    {
        if (data.count(shortName))
            cout << endl << "This short name is already used :(" << endl;
        else {
            Member member(move(shortName), move(role), move(rubric), frequency, move(startDate));
            data[member.GetShortName()] = move(member);
        }
    }

    void AddPost(const string &shortName, const string &date, string &link)
    {
        data.at(shortName).AddPost(date, link);
    }

    bool TalkToUser()
    {
        cout << endl << "Hello! What would u like to do?" << endl;
        cout << "0. I would like to add new member" << endl;
        cout << "1. I would like to add a post for a member" << endl;
        cout << "2. I would like to learn something about members" << endl;
        cout << "3. I would like to learn something about roles" << endl;
        cout << "4. I would like to learn something about rubrics" << endl;
        cout << "5. I would like to delete a post from a member" << endl;
        cout << "6. I would like to delete a member" << endl;
        cout << "7. I would like to change member data" << endl;
        cout << "8. I would like to quit" << endl;
        cout << endl << "Input appropriate number =)" << endl;

        int decision;
        cin >> decision;
        switch (decision) {
            case 0:
                addMember();
                break;
            case 1:
                addPost();
                break;
            case 2:
                learnAboutMembers();
                break;
            case 3:
                learnAboutRoles();
                break;
            case 4:
                learnAboutRubrics();
                break;
            case 5:
                deletePost();
                break;
            case 6:
                deleteMember();
                break;
            case 7:
                changeMemberData();
                break;
            case 8:
                return false;
            default:
                cout << endl << "You made some mistake :(" << endl;
                break;
        }

        cout << endl << "Would u like to do something else?" << endl;
        return bool(AskForDecision());
    }

    int AskForDecision()
    {
        int decision;
        cout << endl << "Input 1 for Yes and 0 for No" << endl;
        cin >> decision;
        return decision;
    }

    string GetFileName() const
    {
        return fileName;
    }

    void FileProblems()
    {
        cout << endl << "Something is wrong with your database file, it should be near your executable & be named "
             << GetFileName() << " :)" << endl << "Proper file is being created now!" << endl;
    }

    ~Database()
    {
        WriteDatabaseToFile();
    }
};

int main(int argc, char **argv)
{
    string regBookName = "../data/Conduit.data";
    if (argc > 1)
        regBookName = argv[1];

    Database regBook(move(regBookName));
    while (regBook.TalkToUser());

    return 0;
}