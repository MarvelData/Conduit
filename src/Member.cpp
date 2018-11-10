#include "Member.hpp"

#include <fstream>

#include "Communication.hpp"
#include "Database.hpp"

using namespace std;

Member::Member() : postsAmount(0), frequency(-1), start("0000.01.01"), changedDeepInfo(false), loadedDeepInfo(false) {}

Member::Member(string &&shortName, string &&role, string &&rubric, int frequency, string &&start) :
shortName(shortName), role(role), rubric(rubric), frequency(frequency), start(start), postsAmount(0), changedDeepInfo(false), loadedDeepInfo(false)
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

void Member::ApprovePost(const string &date, int index)
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

void Member::DeletePost(const string &date, int index)
{
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
            changedDeepInfo = true;
        } else {
            cout << endl << "There is no such post ..." << endl;
            return;
        }
    } else
        Date::DateProblems();
}

int Member::GetPostsAmountAtDate(const string &date) const
{
    if (Date::CheckDate(date)) {
        if (posts.count(date))
            return int(posts.at(date).size());
        else
            return 0;
    }
    Date::DateProblems();
    return -1;
}

vector<TwoStrings> Member::GetPostsAtDate(const string &date) const
{
    if (Date::CheckDate(date)) {
        return posts.at(date);
    }
    Date::DateProblems();
    return vector<TwoStrings>();
}

std::vector<TwoStrings> Member::GetAllPosts() const
{
    vector<TwoStrings> allPosts;

    for (auto &dates : posts)
        allPosts.insert(allPosts.end(), dates.second.begin(), dates.second.end());

    return allPosts;
}

vector<PostInfo> Member::GetNotApprovedPosts() const
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

void Member::PrintPosts(ostream &os) const
{
    for (auto &post : posts) {
        os << post.first << ' ' << post.second.size() << ' ';
        for (auto &link : post.second)
            os << link.first << ' ' << link.second << ' ';
        os << '\\' << endl;
    }
}

void Member::PrintInfo() const
{
    cout << endl << "Short name: " << shortName << endl << "Role: " << role << endl << "Rubric: " << rubric
         << endl << "Frequency (days needed for 1 post): " << frequency << endl << "Start date: " << start << endl;
}

void Member::Rename(const string &newShortName) { shortName = newShortName; changedDeepInfo = true; }

void Member::ChangeRole(const string &newRole) { role = newRole; changedDeepInfo = true; }

void Member::ChangeRubric(const string &newRubric)
{
    rubricSwitches[Date::Now()] = rubric + " changed to " + newRubric;
    rubric = newRubric;
    changedDeepInfo = true;
}

void Member::ChangeFrequency(int newFrequency)
{
    frequencySwitches[Date::Now()] = pair<int, int>(frequency, newFrequency);
    frequency = newFrequency;
    changedDeepInfo = true;
}

void Member::AddVacation(const string &startDate, const string &endDate) { vacations[startDate] = endDate; }

void Member::ForceDeepInfoUpdate() { changedDeepInfo = true; }

string Member::GetShortName() const { return shortName; }

string Member::GetRole() const { return role; }

string Member::GetRubric() const { return rubric; }

int Member::GetFrequency() const { return frequency; }

Date Member::GetStartDate() const { return start; }

int Member::GetPostsAmount() const {
    int counter = 0;
    for (auto &post : posts)
        counter += post.second.size();
    if (counter != postsAmount)
        cout << endl << "DATABASE HAS PROBLEMS!!! AT LEAST WITH POSTS AMOUNTS!" << endl;
    return postsAmount;
}

size_t Member::GetPostsDatesAmount() const { return posts.size(); }

int Member::GetVacationLength(const string &startDate, const string &endDate) const
{
    int length = 0;
    for (auto &vacation : vacations)
        if (vacation.first <= endDate && vacation.second >= startDate)
            length += Date(min(vacation.second, endDate)).Since(Date(max(vacation.first, startDate)));
    return length;
}

bool Member::OnVacation() const
{
    if (vacations.empty())
        return false;
    else if (vacations.rbegin()->second >= Date::Now())
        return true;
    return false;
}

void Member::EndVacation()
{
    vacations.rbegin()->second = Date::Yesterday();
    changedDeepInfo = true;
}

int Member::GetAnticipatedPostsAmount() const
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

void Member::ReadSpecificInfo(const string &path)
{
    if (loadedDeepInfo)
        return;
    ifstream file(path + shortName + ".md");

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

    loadedDeepInfo = true;
}

void Member::PrintSpecificInfo(ostream &os, bool dismission) const
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

bool Member::ChangedDeepInfo() const { return changedDeepInfo; }