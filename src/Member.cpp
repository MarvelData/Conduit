#include "Member.hpp"

#include <fstream>

#include "Communication.hpp"
#include "Database.hpp"

using namespace std;

Member::Member() : postsAmount(0), frequency(-1), start("0000.01.01"), changedDeepInfo(false), loadedDeepInfo(false) {}

Member::Member(string &&shortName, string &&id, string &&role, string &&rubric, int frequency, string &&start, string &&path) :
shortName(shortName), id(id), role(role), rubric(rubric), frequency(frequency), start(start),
postsAmount(0), postsDatesAmount(0), changedDeepInfo(false), loadedDeepInfo(false), path(path)
{
    if (shortName.find(' ') != -1 || role.find(' ') != -1 || rubric.find(' ') != -1) {
        cout << endl << "Short names, roles and rubrics can't contain spaces, srry :D" << endl;
        throw exception();
    }
}

bool Member::AddPost(const string &date, const string &link, const string &status)
{
    string finalLink = Communication::ProceedLink(link);
    if (Date::CheckDate(date)) {
        if (!status.empty())
            posts[date].emplace_back(TwoStrings(finalLink, status));
        else
            posts[date].emplace_back(TwoStrings(finalLink, "-"));
        postsAmount++;
        return true;
    } else {
        Date::DateProblems();
        return false;
    }
}

bool Member::AddPostLight(const string &date, const string &status)
{
    if (Date::CheckDate(date)) {
        if (!status.empty())
            lightPosts[date].emplace_back(TwoStrings("", status));
        else
            lightPosts[date].emplace_back(TwoStrings("", status));
        return true;
    } else {
        Date::DateProblems();
        return false;
    }
}

void Member::SetPostStatus(const string &date, int index, char status)
{
    ForceDeepInfoUpdate();
    if (Date::CheckDate(date)) {
        if (posts.count(date)) {
            if (index >= posts[date].size() || index < 0) {
                cout << endl << "There is no such post ..." << endl;
                return;
            }
            posts[date][index].second = string(1, status);
        } else {
            cout << endl << "There is no such post ..." << endl;
            return;
        }
    } else
        Date::DateProblems();
}

void Member::DeletePost(const string &date, int index)
{
    ForceDeepInfoUpdate();
    if (Date::CheckDate(date)) {
        if (posts.count(date)) {
            if (index >= int(posts[date].size()) || (index < 0 && index != -2)) {
                cout << endl << "There is no such post ..." << endl;
                return;
            }
            if (posts[date].size() > 1)
                posts[date].erase(posts[date].begin() + index);
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

int Member::GetPostsAmountAtDate(const string &date)
{
    ReadSpecificInfo(path);
    if (Date::CheckDate(date)) {
        if (posts.count(date))
            return int(posts.at(date).size());
        else
            return 0;
    }
    Date::DateProblems();
    return -1;
}

vector<TwoStrings> Member::GetPostsAtDate(const string &date)
{
    ReadSpecificInfo(path);
    if (Date::CheckDate(date)) {
        return posts.at(date);
    }
    Date::DateProblems();
    return vector<TwoStrings>();
}

std::vector<TwoStrings> Member::GetAllPosts()
{
    ReadSpecificInfo(path);
    vector<TwoStrings> allPosts;

    for (auto &dates : posts)
        allPosts.insert(allPosts.end(), dates.second.begin(), dates.second.end());

    return allPosts;
}

vector<PostInfo> Member::GetPostsWithStatus(char status)
{
    ReadSpecificInfo(path);
    vector<PostInfo> postsWithSomeStatus;
    for (auto &post : posts) {
        int counter = 0;
        for (auto &link : post.second) {
            if (link.second == string(1, status))
                postsWithSomeStatus.emplace_back(PostInfo(shortName, post.first, link.first, counter));
            counter++;
        }
    }
    return postsWithSomeStatus;
}

void Member::PrintPosts(ostream &os)
{
    ReadSpecificInfo(path);
    for (auto &post : posts) {
        os << post.first << ' ' << post.second.size() << ' ';
        for (auto &link : post.second)
            os << link.first << ' ' << link.second << ' ';
        os << '\\' << endl;
    }
}

void Member::PrintPostsLight(ostream &os)
{
    map<std::string, std::vector<TwoStrings>> lastPosts;
    if (!posts.empty())
        lightPosts = posts;
    int counter = 0;
    for (auto post = lightPosts.rbegin(); counter++ < GetLastDatesAmount(); post++)
        lastPosts.insert(*post);
    for (auto &post : lastPosts)
        os << post.first << ' ';
    os << endl;
    for (auto &post : lastPosts) {
        for (auto &link : post.second)
            os << link.second;
        string buf(post.first.size() - post.second.size(), ' ');
        os << buf << ' ';
    }
    os << endl;
}

void Member::PrintInfo() const
{
    cout << endl << "Short name: " << shortName << endl << "Role: " << role << endl << "Rubric: " << rubric
         << endl << "Frequency (days needed for 1 post): " << frequency << endl << "Start date: " << start << endl;
}

void Member::Rename(const string &newShortName) { shortName = newShortName; ForceDeepInfoUpdate(); }

void Member::ChangeRole(const string &newRole) { role = newRole; ForceDeepInfoUpdate(); }

void Member::ChangeRubric(const string &newRubric)
{
    rubricSwitches[Date::Now()] = rubric + " changed to " + newRubric;
    rubric = newRubric;
    ForceDeepInfoUpdate();
}

void Member::ChangeFrequency(int newFrequency)
{
    frequencySwitches[Date::Now()] = pair<int, int>(frequency, newFrequency);
    frequency = newFrequency;
    ForceDeepInfoUpdate();
}

void Member::AddVacation(const string &startDate, const string &endDate)
{
    if (vacations.count(startDate)) {
        cout << endl << "This vacation already exists, you can only edit it!" << endl;
        return;
    }
    vacations[startDate] = endDate;
}

void Member::EditVacation(const string &startDate, const string &endDate)
{
    if (!vacations.count(startDate)) {
        cout << endl << "This vacation doesn't exist, but you can create it!" << endl;
        return;
    }
    vacations[startDate] = endDate;
}

void Member::ForceDeepInfoUpdate()
{
    changedDeepInfo = true;
    ReadSpecificInfo(path);
}

string Member::GetShortName() const { return shortName; }

string Member::GetRole() const { return role; }

string Member::GetRubric() const { return rubric; }

int Member::GetFrequency() const { return frequency; }

Date Member::GetStartDate() const { return start; }

void Member::SetPostsAmount(int amount) { postsAmount = amount; }

void Member::SetPostsDatesAmount(int amount) { postsDatesAmount = amount; }

int Member::GetPostsAmount() const { return postsAmount; }

size_t Member::GetPostsDatesAmount() const
{
    if (!posts.empty())
        return posts.size();
    if (postsAmount == 0)
        return 0;
    return size_t(postsDatesAmount);
}

int Member::GetLastDatesAmount() const { return min(8, int(GetPostsDatesAmount())); }

int Member::GetVacationLength(const string &startDate, const string &endDate)
{
    ReadSpecificInfo(path);
    int length = 0;
    for (auto &vacation : vacations)
        if (vacation.first <= endDate && vacation.second >= startDate)
            length += Date(min(vacation.second, endDate)).Since(Date(max(vacation.first, startDate)));
    return length;
}

bool Member::OnVacation()
{
    ReadSpecificInfo(path);
    if (vacations.empty())
        return false;
    else if (vacations.rbegin()->first <= Date::Now() && vacations.rbegin()->second >= Date::Now())
        return true;
    return false;
}

void Member::EndVacation()
{
    vacations.rbegin()->second = Date::Yesterday();
    ForceDeepInfoUpdate();
}

int Member::GetAnticipatedPostsAmount(const string &date)
{
    ReadSpecificInfo(path);
    int anticipatedPostsAmount = 0;
    Date lastDate = start;
    for (auto &frequencySwitch : frequencySwitches) {
        anticipatedPostsAmount += (Date(frequencySwitch.first).Since(lastDate)
                                   - GetVacationLength(lastDate.GetAsString(), frequencySwitch.first))
                                  / frequencySwitch.second.first;
        lastDate = Date(frequencySwitch.first);
    }
    auto finalDate = date;
    if (finalDate.empty())
        finalDate = Date::Now();
    anticipatedPostsAmount += (Date(finalDate).Since(lastDate)
                               - GetVacationLength(lastDate.GetAsString(), finalDate))
                              / frequency;
    return anticipatedPostsAmount;
}

void Member::ReadSpecificInfo(const string &path)
{
    if (loadedDeepInfo)
        return;
    ifstream file(path + shortName + ".md");

    if (!file.is_open())
        return;

    string buf;
    int rubricSwitchesAmount, frequencySwitchesAmount, vacationsAmount, postsDatesAmount;

    file >> buf >> buf >> buf >> buf >> buf;
    file >> id;
    id = id.substr(0, id.size() - 1);

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

    file >> buf >> buf >> buf >> buf >> buf >> buf >> buf;
    file >> postsDatesAmount;
    file >> buf;
    if (postsDatesAmount != this->postsDatesAmount)
        cout << endl << "DATABASE HAS PROBLEMS!!! AT LEAST WITH DEEP DATA SYNCHRONIZATION!" << endl;
    for (int j = 0; j < postsDatesAmount; j++) {
        string date, link;
        int amount;
        bool acquireStatus = false;
        file >> date;
        file >> amount;
        if (lightPosts.count(date))
            acquireStatus = true;
        for (int k = 0; k < amount; k++) {
            file >> link >> buf;
            if (Date::CheckDate(date)) {
                if (acquireStatus) buf = lightPosts[date][k].second;
                posts[date].emplace_back(TwoStrings(link, buf));
            } else
                Date::DateProblems();
        }
        file >> buf;
    }

    file.close();

    loadedDeepInfo = true;
}

void Member::PrintSpecificInfo(ostream &os) const
{
    os << id << "\\" << endl;

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
}

bool Member::ChangedDeepInfo() const { return changedDeepInfo; }