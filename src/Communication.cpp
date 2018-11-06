#include "Communication.hpp"

#include <iostream>

#include "Database.hpp"

using namespace std;

void Communication::changeMemberData()
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
    cout << i++ << divider << "I would like to approve post" << endl;
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
            changeMemberData();
            break;
        case 8:
            database->ApprovePost();
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
    Communication::Tabulator(to_string(counter) + ". " + member.GetShortName(), 16);
    if (moreInfo) {
        cout << "Rubric: "  << member.GetRubric() << '\t';
        Communication::Tabulator(member.GetRubric(), 8);
        cout << "Frequency: " << member.GetFrequency() << '\t';
        database->PrintPostsAmounts(member.GetShortName());
    }
    else
        cout << endl;
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
    if (finalLink.find("mu_marveluniverse") == -1)
        finalLink = finalLink.substr(0, 15) + "mu_marveluniverse?w=" + finalLink.substr(15);
    return finalLink;
}

int Communication::AskForDecision()
{
    int decision;
    cout << endl << "Input 1 for Yes and 0 for No" << endl;
    cin >> decision;
    return decision;
}