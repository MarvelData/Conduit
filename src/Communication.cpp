#include "Communication.hpp"

#include <iostream>
#include <cmath>

#include "Database.hpp"
#include "Math.hpp"

using namespace std;

void Communication::changeMemberDataDialog()
{
    database->PrintMembers();

    int i = 0;
    string divider = ". ";
    cout << endl << "What would u like to change?" << endl;
    cout << i++ << divider << "I would like to change member short name" << endl;
    cout << i++ << divider << "I would like to change member role" << endl;
    cout << i++ << divider << "I would like to change member rubric" << endl;
    cout << i++ << divider << "I would like to change member frequency" << endl;
    cout << i++ << divider << "I would like to add vacation for member" << endl;
    cout << endl << "Input appropriate number or -1 to return =)" << endl;

    cin >> i;
    switch (i) {
        case -1:
            break;
        case 0:
            database->ChangeMemberShortName();
            break;
        case 1:
            database->ChangeMemberRole();
            break;
        case 2:
            database->ChangeMemberRubric();
            break;
        case 3:
            database->ChangeMemberFrequency();
            break;
        case 4:
            database->AddVacation();
            break;
        default:
            cout << endl << "You made some mistake :(" << endl;
            break;
    }
}

void Communication::managePostsStatusesDialog()
{
    int i = 0;
    string divider = ". ";
    cout << endl << "What would u like to do?" << endl;
    cout << i++ << divider << "I would like to approve post" << endl;
    cout << i++ << divider << "I would like to reject post" << endl;
    cout << i++ << divider << "I would like to see approved posts" << endl;
    cout << i++ << divider << "I would like to see rejected posts" << endl;
    cout << endl << "Input appropriate number or -1 to return =)" << endl;

    cin >> i;
    switch (i) {
        case -1:
            break;
        case 0:
            database->ApprovePost();
            break;
        case 1:
            database->RejectPost();
            break;
        case 2:
            database->GetPostsWithStatus('+');
            break;
        case 3:
            database->GetPostsWithStatus('!');
            break;
        default:
            cout << endl << "You made some mistake :(" << endl;
            break;
    }
}

Communication::Communication(Database *database) : database(database) {}

void Communication::FileProblems()
{
    cout << endl << "Something is wrong with your database file, it should be near your executable & be named "
         << database->GetFileName() << " :)" << endl << "Proper file is being created now!" << endl;
}

bool Communication::TalkToUser()
{
    int i = 0;
    string divider = ".  ";
    cout << endl << "Hello! What would u like to do?" << endl;
    cout << i++ << divider << "I would like to add new member" << endl;
    cout << i++ << divider << "I would like to add a post for a member" << endl;
    cout << i++ << divider << "I would like to learn something about members" << endl;
    cout << i++ << divider << "I would like to learn something about roles" << endl;
    cout << i++ << divider << "I would like to learn something about rubrics" << endl;
    cout << i++ << divider << "I would like to delete a post from a member" << endl;
    cout << i++ << divider << "I would like to delete a member" << endl;
    cout << i++ << divider << "I would like to change member data" << endl;
    cout << i++ << divider << "I would like to manage posts statuses" << endl;
    cout << i++ << divider << "I would like to update deep data for all members" << endl;
    divider = ". ";
    cout << i++ << divider << "I would like to check duplicates" << endl;
    cout << i++ << divider << "I would like to find post (posts)" << endl;
    cout << endl << "Input appropriate number or -1 to exit =)" << endl;

    cin >> i;
    switch (i) {
        case -1:
            return false;
        case 0:
            database->AddMember();
            break;
        case 1:
            database->AddPost();
            break;
        case 2:
            database->LearnAboutMembers();
            break;
        case 3:
            database->LearnAboutRoles();
            break;
        case 4:
            database->LearnAboutRubrics();
            break;
        case 5:
            database->DeletePost();
            break;
        case 6:
            database->DeleteMember();
            break;
        case 7:
            changeMemberDataDialog();
            break;
        case 8:
            managePostsStatusesDialog();
            break;
        case 9:
            database->UpdateDeepInfo();
            break;
        case 10:
            database->CheckDuplicates();
            break;
        case 11:
            database->FindPosts();
            break;
        default:
            cout << endl << "You made some mistake :(" << endl;
            break;
    }

    cout << endl << "Would u like to do something else?" << endl;
    return bool(AskForDecision());
}

void Communication::PrintMember(Member &member, int counter, bool moreInfo)
{
    cout << counter << ". " << member.GetShortName() << '\t';
    Tabulator(to_string(counter) + ". " + member.GetShortName(), 16);
    if (moreInfo) {
        cout << "Rubric: "  << member.GetRubric() << '\t';
        Tabulator(member.GetRubric(), 8);
        cout << "Frequency: " << member.GetFrequency() << '\t';
        database->PrintPostsAmounts(member.GetShortName());
    }
    else
        cout << endl;
}

void Communication::PrintRoles(const map<string, vector<string>> &roles)
{
    cout << endl << "Current roles list:" << endl;
    int counter = 0;
    for (auto &role : roles)
        cout << counter << ". " << role.first << endl;
}

void Communication::PrintRubrics(const map<string, vector<int>> &rubrics)
{
    vector<double> statistics;
    for (auto &rubric : rubrics) {
        int leastCommonMultiplier = 1, sum = 0;
        for (auto frequency : rubric.second)
            leastCommonMultiplier = Math::LeastCommonMultiplier(leastCommonMultiplier, frequency);
        for (auto frequency : rubric.second)
            sum += leastCommonMultiplier / frequency;
        statistics.emplace_back(double(leastCommonMultiplier) / sum);
    }

    cout << endl << "Current rubrics list:" << endl;
    int counter = 0;
    for (auto &rubric : rubrics) {
        cout << counter << ". " << rubric.first << ":\t";
        Tabulator(to_string(counter) + ". " + rubric.first, 7);
        cout << "Editors amount: " << rubric.second.size()
             << "\tOverall frequency (days needed for 1 post): " << statistics[counter]
             << " (" << ceil(statistics[counter]) << ')' << endl;
        counter++;
    }
}

string Communication::CollectNewMemberName(const string &oldShortName)
{
    string shortName;
    cin >> shortName;
    if (shortName.empty()) {
        cout << endl << "You can't use empty short name :(" << endl;
        return "";
    }
    if (shortName == oldShortName) {
        cout << endl << "U made no changes :D" << endl;
        return "";
    }
    if (database->ContainsMember(shortName)) {
        cout << endl << "This short name is already used" << endl;
        return "";
    }
    return shortName;
}

PostInfo Communication::CollectPostInfo(bool deleting)
{
    string shortName, date, link;
    int index = -1;
    shortName = database->CollectMemberName();
    if (shortName.empty())
        return PostInfo();
    date = Date::CollectDate();
    if (deleting) {
        int amount = database->GetPostsAmount(shortName, date);
        if (amount == 1)
            index = -2;
        else if (amount > 1) {
            cout << endl << "Input index: " << endl;
            cin >> index;
        }
    } else {
        cout << endl << "Input link: " << endl;
        cin >> link;
    }
    return PostInfo(shortName, date, link, index);
}

void Communication::Tabulator(const string &str, int threshold)
{
    if (str.size() < threshold)
        cout << '\t';
}

string Communication::ProceedLink(const string &link)
{
    string finalLink = link;
    if (link.find("http") == -1)
        finalLink = "https://" + link;
    if (finalLink.find("mu_marvel") == -1)
        if (link.size() > 25)
            finalLink = finalLink.substr(0, 15) + "mu_marvel?w=" + finalLink.substr(15);
        else
            cout << endl << "Warning: your link seems to be incorrect!" << endl;
    return finalLink;
}

int Communication::AskForDecision()
{
    int decision;
    cout << endl << "Input 1 for Yes and 0 for No" << endl;
    cin >> decision;
    return decision;
}