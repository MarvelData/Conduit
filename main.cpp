#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <random>
#include <stdexcept>

using namespace std;

typedef pair<string, string> TwoStrings;

string GetRandomString(int n)
{
    string result;
    for (size_t i = 0; i < n; i++)
        result += char(rand() % ('z' - 'a' + 1) + 'a');
    return result;
}

int GreatestCommonDivisor(int a, int b) { return b ? GreatestCommonDivisor(b, a % b) : a; }

int LeastCommonMultiplier(int a, int b) { return a / GreatestCommonDivisor(a, b) * b; }

class StreamControl
{
    ofstream out;

public:
    void Redirect(const string &name = "TempRegBookOut.txt")
    {
        out.open(name);
        cout.rdbuf(out.rdbuf());
    }

    ~StreamControl() { out.close(); }
};

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

    int Since(const Date &oldDate)
    {
        if (oldDate.year > year) {
            cout << endl << "Wrong dates order!" << endl;
            return -1;
        }
        int diff = 0;
        if (year == oldDate.year) {
            if (oldDate.month > month) {
                cout << endl << "Wrong dates order!" << endl;
                return -1;
            }
            if (month == oldDate.month) {
                if (oldDate.day > day) {
                    cout << endl << "Wrong dates order!" << endl;
                    return -1;
                }
                return day - oldDate.day;
            }
            for (int i = oldDate.month + 1; i < month; i++)
                diff += GetDaysInMonth(year, i);
        } else {
            for (int i = oldDate.year + 1; i < year; i++)
                for (int j = 1; j <= 12; j++)
                    diff += GetDaysInMonth(i, j);
            for (int i = oldDate.month + 1; i <= 12; i++)
                diff += GetDaysInMonth(oldDate.year, i);
            for (int i = 1; i < month; i++)
                diff += GetDaysInMonth(year, i);

        }
        return diff + GetDaysInMonth(oldDate.year, oldDate.month) - oldDate.day + day;
    }

    string GetAsString() const { return date; }

    static int GetDaysInMonth(int year, int month)
    {
        switch (month)
        {
            case 1:
                return 31;
            case 2:
                if (year % 4)
                    return 28;
                return 29;
            case 3:
                return 31;
            case 4:
                return 30;
            case 5:
                return 31;
            case 6:
                return 30;
            case 7:
                return 31;
            case 8:
                return 31;
            case 9:
                return 30;
            case 10:
                return 31;
            case 11:
                return 30;
            case 12:
                return 31;
            default:
                cout << endl << "There is some mistake with date management:(" << endl;
                return -1;
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

    static string CollectDate(bool unlimitedPossibility = false)
    {
        string date;
        bool correct = true;
        do {
            if (!correct)
                DateProblems();
            cout << endl << "Input date in format YYYY.MM.DD (to use current date, just input 0, yesterday : -1): " << endl;
            cin >> date;
            if (unlimitedPossibility && date == "unlimited")
                return date;
            if (date == "0")
                return Now();
            if (date == "-1")
                return Yesterday();
        } while (!(correct = CheckDate(date)));
        return date;
    }

    static string TimeToDate(const time_t &time)
    {
        string date;
        date = to_string(localtime(&time)->tm_year + 1900) + '.';
        if (localtime(&time)->tm_mon + 1 < 10)
            date += '0';
        date += to_string(localtime(&time)->tm_mon + 1) + '.';
        if (localtime(&time)->tm_mday < 10)
            date += '0';
        date += to_string(localtime(&time)->tm_mday);
        return date;
    }

    static string Now() { return TimeToDate(chrono::system_clock::to_time_t(chrono::system_clock::now())); }

    static string Yesterday() { return TimeToDate(chrono::system_clock::to_time_t(chrono::system_clock::now()) - 86400); }

    friend ostream& operator<<(ostream& os, const Date& date);
};

ostream& operator<<(ostream& os, const Date& date)
{
    os << date.date;
    return os;
}

struct PostInfo
{
    string ShortName;
    string Date;
    string Link;
    int Index;
    bool Correct;

    PostInfo() : Index(-1), Correct(false) {}
    PostInfo(const string &shortName, const string &date, const string &link, int index) :
            ShortName(shortName), Date(date), Link(link), Index(index), Correct(true)
    {
        if (ShortName.find(' ') != -1 || Date.find(' ') != -1 || Link.find(' ') != -1) {
            cout << endl << "Short names, roles and rubrics can't contain spaces, srry :D" << endl;
            throw exception();
        }
    }
};

class Member {
    string shortName;
    string role;
    string rubric;
    int frequency;
    Date start;
    map<string, string> rubricSwitches;
    map<string, pair<int, int>> frequencySwitches;
    map<string, string> vacations;
    int postsAmount;
    map<string, vector<TwoStrings>> posts;
    bool changedDeepInfo;

public:
    Member() : postsAmount(0), frequency(-1), start("0000.01.01"), changedDeepInfo(false) {}

    Member(string &&shortName, string &&role, string &&rubric, int frequency, string &&start) :
            shortName(shortName), role(role), rubric(rubric), frequency(frequency), start(start), postsAmount(0), changedDeepInfo(false)
    {
        if (shortName.find(' ') != -1 || role.find(' ') != -1 || rubric.find(' ') != -1) {
            cout << endl << "Short names, roles and rubrics can't contain spaces, srry :D" << endl;
            throw exception();
        }
    }

    bool AddPost(const string &date, const string &link, bool approved = false)
    {
        string finalLink = link;
        if (link.find("http") == -1)
            finalLink = "https://" + link;
        if (finalLink.find("mu_marveluniverse") == -1)
            finalLink = finalLink.substr(0, 15) + "mu_marveluniverse?w=" + finalLink.substr(15);
        if (Date::CheckDate(date)) {
            if (approved)
                posts[date].emplace_back(TwoStrings(finalLink, "+"));
            else
                posts[date].emplace_back(TwoStrings(finalLink, "-"));
            postsAmount++;
            return true;
        } else {
            Date::DateProblems();
            return false;
        }
    }

    void ApprovePost(const string &date, int index)
    {
        if (Date::CheckDate(date)) {
            if (posts.count(date)) {
                if (index >= posts[date].size() || index < 0) {
                    cout << endl << "There is no such post ..." << endl;
                    return;
                }
                posts[date][index].second = "+";
            } else {
                cout << endl << "There is no such post ..." << endl;
                return;
            }
        } else
            Date::DateProblems();
    }

    void DeletePost(const string &date, int index)
    {
        if (Date::CheckDate(date)) {
            if (posts.count(date)) {
                if (index >= posts[date].size() || index < 0) {
                    cout << endl << "There is no such post ..." << endl;
                    return;
                }
                if (posts[date].size() > 1)
                    posts[date].erase(posts[date].begin() + index);
                else
                    posts.erase(date);
                postsAmount--;
                changedDeepInfo = true;
            } else {
                cout << endl << "There is no such post ..." << endl;
                return;
            }
        } else
            Date::DateProblems();
    }

    vector<TwoStrings> GetPostsAtDate(const string &date) const
    {
        if (Date::CheckDate(date)) {
            return posts.at(date);
        }
        Date::DateProblems();
        return vector<TwoStrings>();
    }

    vector<PostInfo> GetNotApprovedPosts() const
    {
        vector<PostInfo> notApprovedPosts;
        for (auto &post : posts) {
            int counter = 0;
            for (auto &link : post.second) {
                if (link.second == "-")
                    notApprovedPosts.emplace_back(PostInfo(shortName, post.first, link.first, counter));
                counter++;
            }
        }
        return notApprovedPosts;
    }

    void PrintPosts(ostream &os) const
    {
        for (auto &post : posts) {
            os << post.first << ' ' << post.second.size() << ' ';
            for (auto &link : post.second)
                os << link.first << ' ' << link.second << '\t';
            os << '\\' << endl;
        }
    }

    void PrintInfo() const
    {
        cout << endl << "Role: " << role << endl << "Rubric: " << rubric
             << endl << "Frequency (days needed for 1 post): " << frequency << endl << "Start date: " << start << endl;
    }

    void Rename(const string &newShortName) { shortName = newShortName; changedDeepInfo = true; }

    void ChangeRole(const string &newRole) { role = newRole; changedDeepInfo = true; }

    void ChangeRubric(const string &newRubric)
    {
        rubricSwitches[Date::Now()] = rubric + " changed to " + newRubric;
        rubric = newRubric;
        changedDeepInfo = true;
    }

    void ChangeFrequency(int newFrequency)
    {
        frequencySwitches[Date::Now()] = pair<int, int>(frequency, newFrequency);
        frequency = newFrequency;
        changedDeepInfo = true;
    }

    void AddVacation(const string &startDate, const string &endDate)
    {
        vacations[startDate] = endDate;
        changedDeepInfo = true;
    }

    void ForceDeepInfoUpdate() { changedDeepInfo = true; }

    string GetShortName() const { return shortName; }

    string GetRole() const { return role; }

    string GetRubric() const { return rubric; }

    int GetFrequency() const { return frequency; }

    Date GetStartDate() const { return start; }

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

    int GetVacationLength(const string &startDate, const string &endDate) const
    {
        int length = 0;
        for (auto &vacation : vacations)
            if (vacation.first <= endDate && vacation.second >= startDate)
                length += Date(min(vacation.second, endDate)).Since(Date(max(vacation.first, startDate)));
        return length;
    }

    bool OnVacation() const
    {
        if (vacations.empty())
            return false;
        else if (vacations.rbegin()->second >= Date::Now())
            return true;
        return false;
    }

    int GetAnticipatedPostsAmount() const
    {
        int anticipatedPostsAmount = 0;
        Date lastDate = start;
        for (auto &frequencySwitch : frequencySwitches) {
            anticipatedPostsAmount += (Date(frequencySwitch.first).Since(lastDate)
                                       - GetVacationLength(lastDate.GetAsString(), frequencySwitch.first))
                                       / frequencySwitch.second.first;
            lastDate = Date(frequencySwitch.first);
        }
        anticipatedPostsAmount += (Date(Date::Now()).Since(lastDate)
                                   - GetVacationLength(lastDate.GetAsString(), Date::Now()))
                                  / frequency;
        return anticipatedPostsAmount;
    }

    void ReadSpecificInfo()
    {
        ifstream file("../data/" + shortName + ".md");

        if (!file.is_open())
            return;

        string buf;
        int rubricSwitchesAmount, frequencySwitchesAmount, vacationsAmount;

        file >> buf >> buf >> buf >> buf >> buf;
        file >> rubricSwitchesAmount;
        file >> buf >> buf;
        for (int i = 0; i < rubricSwitchesAmount; i++) {
            string date, oldRubric, newRubric, info;
            file >> date >> oldRubric >> info >> buf >> newRubric;
            rubricSwitches[date] = oldRubric.append(' ' + info).append(' ' + buf).append(' ' + newRubric);
            file >> buf;
        }

        file >> frequencySwitchesAmount;
        file >> buf >> buf;
        for (int i = 0; i < frequencySwitchesAmount; i++) {
            string date;
            int oldFrequency, newFrequency;
            file >> date >> oldFrequency >> buf >> buf;
            file >> newFrequency >> buf;
            frequencySwitches[date].first = oldFrequency;
            frequencySwitches[date].second = newFrequency;
        }

        file >> vacationsAmount;
        file >> buf;
        for (int i = 0; i < vacationsAmount; i++) {
            string startDate, endDate;
            file >> startDate >> endDate >> buf;
            AddVacation(startDate, endDate);
        }

        file.close();
    }

    void PrintSpecificInfo(ostream &os, bool dismission = false) const
    {
        os << rubricSwitches.size() << " rubric switches:" << "\\" << endl;
        for (auto &rubricSwitch : rubricSwitches)
            os << rubricSwitch.first << ' ' << rubricSwitch.second << " \\" << endl;

        os << frequencySwitches.size() << " frequency switches:" << "\\" << endl;
        for (auto &frequencySwitch : frequencySwitches)
            os << frequencySwitch.first << ' ' << frequencySwitch.second.first
               << " changed to " << frequencySwitch.second.second << " \\" << endl;

        os << vacations.size() << " vacations:" << "\\" << endl;
        for (auto &vacation : vacations)
            os << vacation.first << ' ' << vacation.second << " \\" << endl;

        if (dismission)
            os << "Finished on " << Date::Now() << "\\" << endl;
    }

    bool ChangedDeepInfo() const { return changedDeepInfo; }
};

class Database
{
    string fileName;
    map<string, Member> data;

    void addMember()
    {
        printMembers();
        string shortName, role, rubric, startDate;
        int frequency;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input member short name: " << endl;
        cin >> shortName;
        if (shortName.empty()) {
            cout << endl << "You can't add empty short name :(" << endl;
            return;
        }
        cout << endl << "Input member role: " << endl;
        cin >> role;
        cout << endl << "Input member rubric: " << endl;
        cin >> rubric;
        cout << endl << "Input frequency (days needed for 1 post, must be positive integer): " << endl;
        cin >> frequency;
        startDate = Date::CollectDate();
        if (data.count(shortName))
            cout << endl << "This short name is already used :(" << endl;
        else {
            Member member(move(shortName), move(role), move(rubric), frequency, move(startDate));
            member.ForceDeepInfoUpdate();
            data[member.GetShortName()] = member;
            cout << endl << "New member is successfully added!" << endl;
        }
        WriteDatabaseToFiles();
    }

    void tabulator(const string &str, int threshold)
    {
        if (str.size() < threshold)
            cout << '\t';
    }

    void printMembers(bool moreInfo = false)
    {
        cout << endl << "Current members list:" << endl;
        int counter = 0;
        for (auto &elem : data) {
            cout << counter << ". " << elem.first << '\t';
            tabulator(to_string(counter++) + ". " + elem.first, 16);
            if (moreInfo) {
                cout << "Rubric: "  << data[elem.first].GetRubric() << '\t';
                data[elem.first].ReadSpecificInfo();
                printPostsAmounts(elem.first);
            }
            else
                cout << endl;
        }
    }

    string collectMemberName(bool moreInfo = false)
    {
        string shortName;
        printMembers(moreInfo);
        bool correct = true;
        do {
            if (!correct)
                cout << endl << "No such member! Try again.";
            bool digits = true;
            cout << endl << "Input member (u can also input according number): " << endl;
            cin >> shortName;
            for (auto c : shortName)
                if (!isdigit(c)) {
                    digits = false;
                    break;
                }
            if (digits) {
                int counter = 0;
                for (auto &elem : data)
                    if (stoi(shortName) == counter++) {
                        shortName = elem.first;
                        break;
                    }
            }
        } while (!(correct = bool(data.count(shortName))));
        cout << endl << "Here are his/her posts: " << endl;
        data.at(shortName).PrintPosts(cout);
        return shortName;
    }

    PostInfo collectPostInfo(bool deleting)
    {
        string shortName, date, link;
        int index = -1;
        shortName = collectMemberName();
        date = Date::CollectDate();
        if (deleting) {
            cout << endl << "Input index: " << endl;
            cin >> index;
        } else {
            cout << endl << "Input link: " << endl;
            cin >> link;
        }
        return PostInfo(shortName, date, link, index);
    }

    void addPost()
    {
        PostInfo postInfo = collectPostInfo(false);
        if (postInfo.Correct)
            data[postInfo.ShortName].AddPost(postInfo.Date, postInfo.Link);
        WriteDatabaseToFiles();
    }

    void approvePost()
    {
        size_t counter = 0;
        vector<PostInfo> posts;
        for (auto &member : data) {
            vector<PostInfo> notApprovedPosts = member.second.GetNotApprovedPosts();
            for (auto &post : notApprovedPosts) {
                cout << counter++ << ". " << post.ShortName << ' ' << post.Date << ' ' << post.Link << endl;
                posts.emplace_back(post);
            }
        }

        cout << "Input number of the post u are approving or -1 to avoid approving: " << endl;
        cin >> counter;

        if (counter == -1)
            return;

        data.at(posts.at(counter).ShortName).ApprovePost(posts.at(counter).Date, posts.at(counter).Index);

        WriteDatabaseToFiles();
    }

    void printPostsAmounts(const string &shortName, ostream &os = cout)
    {
        int postsAmount = data[shortName].GetPostsAmount();
        int anticipatedPostsAmount = data[shortName].GetAnticipatedPostsAmount();
        os << "Actual posts amount: " << postsAmount;
        os << "\tAnticipated posts amount: " << anticipatedPostsAmount;
        if (anticipatedPostsAmount - postsAmount > 1 & !data[shortName].OnVacation())
            os << "\t Lag: " << anticipatedPostsAmount - postsAmount;
        os << endl;
    }

    void learnAboutMembers()
    {
        string shortName = collectMemberName(true);
        data[shortName].PrintInfo();
        data[shortName].PrintSpecificInfo(cout);
        printPostsAmounts(shortName);
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
            elem.second.ReadSpecificInfo();
            if (!elem.second.OnVacation()) {
                if (rubrics.count(elem.second.GetRubric()))
                    rubrics[elem.second.GetRubric()].first++;
                else
                    rubrics[elem.second.GetRubric()] = pair<int, vector<int>>(1, vector<int>());
                rubrics[elem.second.GetRubric()].second.emplace_back(elem.second.GetFrequency());
            }
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
            cout << rubric.first << ":\t";
            tabulator(rubric.first, 7);
            cout << "Editors amount: " << rubric.second.first
                 << "\tOverall frequency (days needed for 1 post): " << statistics[counter]
                 << " (" << ceil(statistics[counter]) << ')' << endl;
            counter++;
        }
    }

    void deletePost()
    {
        PostInfo postInfo = collectPostInfo(true);
        if (postInfo.Correct)
            data[postInfo.ShortName].DeletePost(postInfo.Date, postInfo.Index);
        WriteDatabaseToFiles();
    }

    void deleteMember()
    {
        string shortName = collectMemberName();
        if (data.count(shortName)) {
            data[shortName].ForceDeepInfoUpdate();
            WriteDatabaseToFiles(true);
            data.erase(shortName);
        } else
            cout << endl << "There is no such member already!" << endl;
        WriteDatabaseToFiles();
    }

    void changeMemberShortName()
    {
        string oldShortName = collectMemberName();
        string newShortName;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input new short name for this member: " << endl;
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
        WriteDatabaseToFiles();
    }

    void changeMemberRole()
    {
        string shortName = collectMemberName();
        data[shortName].PrintInfo();
        string newRole;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input new role for this member: " << endl;
        cin >> newRole;
        data[shortName].ChangeRole(newRole);
        WriteDatabaseToFiles();
    }

    void changeMemberRubric()
    {
        string shortName = collectMemberName();
        data[shortName].PrintInfo();
        string newRubric;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input new rubric for this member: " << endl;
        cin >> newRubric;
        data[shortName].ChangeRubric(newRubric);
        WriteDatabaseToFiles();
    }

    void changeMemberFrequency()
    {
        string shortName = collectMemberName();
        data[shortName].PrintInfo();
        int newFrequency;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input new frequency for this member (days needed for 1 post, must be positive integer): " << endl;
        cin >> newFrequency;
        data[shortName].ChangeFrequency(newFrequency);
        WriteDatabaseToFiles();
    }

    void addVacation()
    {
        string shortName = collectMemberName();
        data[shortName].PrintInfo();
        string start, end;
        cout << endl << "NB: Input without spaces!" << endl;
        cout << endl << "Input vacation start date: " << endl;
        start = Date::CollectDate();
        cout << endl << "Input vacation end date or \"unlimited\": " << endl;
        end = Date::CollectDate(true);
        data[shortName].AddVacation(start, end);
        WriteDatabaseToFiles();
    }

    void changeMemberData()
    {
        printMembers();
        cout << endl << "What would u like to change?" << endl;
        cout << "0. I would like to change member short name" << endl;
        cout << "1. I would like to change member role" << endl;
        cout << "2. I would like to change member rubric" << endl;
        cout << "3. I would like to change member frequency" << endl;
        cout << "4. I would like to add vacation for member" << endl;
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
            case 4:
                addVacation();
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
            this->fileName = GetFileName();
            ifstream tryFile(this->fileName);
            if (!tryFile.is_open()) {
                FileProblems();
                ofstream newFile(this->fileName);
                newFile.close();
                return;
            }
            tryFile.close();
            file.open(this->fileName);
        }

        string buf;
        int membersAmount;
        file >> buf >> buf;
        file >> membersAmount;
        file >> buf >> buf;
        for (int i = 0; i < membersAmount; i++) {
            string shortName, role, rubric, startDate;
            int frequency, postsDatesAmount;
            file >> shortName >> role >> rubric;
            file >> frequency;
            file >> startDate;
            startDate = startDate.substr(0, startDate.size() - 1);
            Member member(move(shortName), move(role), move(rubric), frequency, move(startDate));
            file >> buf >> buf >> buf >> buf >> buf >> buf >> buf;
            file >> postsDatesAmount;
            file >> buf;
            for (int j = 0; j < postsDatesAmount; j++) {
                string date, link;
                int amount;
                file >> date;
                file >> amount;
                for (int k = 0; k < amount; k++) {
                    file >> link >> buf;
                    if (buf == "+")
                        member.AddPost(date, link, true);
                    else
                        member.AddPost(date, link);
                }
                file >> buf;
            }
            file >> buf;
            data[member.GetShortName()] = move(member);
        }

        file.close();
        for (auto &elem : data)
            elem.second.GetPostsAmount();
    }

    void WriteDatabaseToFiles(bool dismission = false)
    {
        for (auto &elem : data)
            elem.second.GetPostsAmount();
        ofstream file(fileName);

        file << "Members amount: " << data.size() << '\\' << endl << '\\' << endl;
        for (auto &elem : data) {
            file << elem.first << '\t' << elem.second.GetRole() << ' ' << elem.second.GetRubric()
                 << ' ' << elem.second.GetFrequency() << ' ' << elem.second.GetStartDate() << '\\' << endl;
            file << "Total posts amount: " << elem.second.GetPostsAmount() << '\t'
                 << "Posts dates amount: " << elem.second.GetPostsDatesAmount() << '\\' << endl;
            elem.second.PrintPosts(file);
            file << '\\' << endl;

            if (elem.second.ChangedDeepInfo()) {
                elem.second.ReadSpecificInfo();
                ofstream memberFile("../data/" + elem.first + ".md");
                memberFile << elem.first << '\t' << elem.second.GetRole() << ' ' << elem.second.GetRubric()
                           << ' ' << elem.second.GetFrequency() << ' ' << elem.second.GetStartDate() << '\\' << endl;
                elem.second.PrintSpecificInfo(memberFile, dismission);
                if (dismission) {
                    printPostsAmounts(elem.first, memberFile);
                    memberFile << "\\" << endl;
                }
                memberFile << "Total posts amount: " << elem.second.GetPostsAmount() << '\t'
                           << "Posts dates amount: " << elem.second.GetPostsDatesAmount() << '\\' << endl;
                elem.second.PrintPosts(memberFile);
                memberFile.close();
                if (dismission) {
                    string oldName = "../data/" + elem.first + ".md",
                           newName = "../data/" + elem.first + "_dismissed.md";
                    rename(oldName.c_str(), newName.c_str());
                }
            }
        }

        file.close();
    }

    void AddMemberTest(string shortName, string role, string rubric, int frequency, string startDate)
    {
        if (data.count(shortName))
            cout << endl << "This short name is already used :(" << endl;
        else {
            Member member(move(shortName), move(role), move(rubric), frequency, move(startDate));
            data[member.GetShortName()] = move(member);
        }
    }

    void AddPostTest(const string &shortName, const string &date, string &link)
    {
        data.at(shortName).AddPost(date, link);
    }

    bool TalkToUser()
    {
        cout << endl << "Hello! What would u like to do?" << endl;
        cout << "0. \t I would like to add new member" << endl;
        cout << "1. \t I would like to add a post for a member" << endl;
        cout << "2. \t I would like to learn something about members" << endl;
        cout << "3. \t I would like to learn something about roles" << endl;
        cout << "4. \t I would like to learn something about rubrics" << endl;
        cout << "5. \t I would like to delete a post from a member" << endl;
        cout << "6. \t I would like to delete a member" << endl;
        cout << "7. \t I would like to change member data" << endl;
        cout << "8. \t I would like to approve post" << endl;
        cout << "9. \t I would like to update deep data" << endl;
        cout << "10. \t I would like to update deep data for all members" << endl;
        cout << endl << "Input appropriate number or -1 to exit =)" << endl;

        int decision;
        cin >> decision;
        switch (decision) {
            case -1:
                return false;
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
                approvePost();
                break;
            case 9:
                data[collectMemberName()].ForceDeepInfoUpdate();
                break;
            case 10:
                for (auto &member : data)
                    member.second.ForceDeepInfoUpdate();
                break;
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
        if (fileName.find('/') == -1)
            return fileName;
        return fileName.substr(fileName.find_last_of('/') + 1);
    }

    void FileProblems()
    {
        cout << endl << "Something is wrong with your database file, it should be near your executable & be named "
             << GetFileName() << " :)" << endl << "Proper file is being created now!" << endl;
    }

    ~Database()
    {
        WriteDatabaseToFiles();
    }
};

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