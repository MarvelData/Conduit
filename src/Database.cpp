#include "Database.hpp"

#include <fstream>

#include "Communication.hpp"

using namespace std;

map<string, vector<string>> Database::getRoles()
{
    map<string, vector<string>> roles;
    for (auto &memberMetaPair : data)
        roles[memberMetaPair.second.GetRole()].emplace_back(memberMetaPair.first);
    return roles;
}

map<string, vector<int>> Database::getRubrics(bool consideringVacations)
{
    map<string, vector<int>> rubrics;
    if (consideringVacations)
        for (auto &memberMetaPair : data) {
            if (!memberMetaPair.second.OnVacation())
                rubrics[memberMetaPair.second.GetRubric()].emplace_back(memberMetaPair.second.GetFrequency());
        }
    else
        for (auto &memberMetaPair : data)
            rubrics[memberMetaPair.second.GetRubric()].emplace_back(memberMetaPair.second.GetFrequency());
    return rubrics;
}

pair<map<string, vector<string>>, int> Database::collectAllPosts()
{
    map<string, vector<string>> posts;

    int allPostsAmount = 0;
    for (auto &member : data) {
        auto memberPosts = member.second.GetAllPosts();
        for (auto &post : memberPosts)
            posts[post.first].emplace_back(member.first);
        allPostsAmount += member.second.GetPostsAmount();
    }

    return make_pair(posts, allPostsAmount);
}

Database::Database(string &&fileName) : fileName(fileName), communicator(nullptr)
{
    ifstream file(fileName);

    if (!file.is_open()) {
        this->fileName = GetFileName();
        ifstream tryFile(this->fileName);
        if (!tryFile.is_open()) {
            communicator->FileProblems();
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
    for (int i = 0; i < membersAmount; i++) {
        string shortName, role, rubric, startDate;
        int frequency, postsAmount, postsDatesAmount;
        size_t lastDatesAmount;
        file >> shortName >> role >> rubric;
        file >> frequency;
        file >> startDate;
        Member member(move(shortName), move(role), move(rubric), frequency, move(startDate), GetPath());
        file >> buf >> buf >> buf;
        file >> postsAmount;
        file >> buf >> buf >> buf;
        file >> postsDatesAmount;
        file >> buf >> buf >> buf;
        file >> lastDatesAmount;
        vector<string> lastDates(lastDatesAmount);
        for (size_t j = 0; j < lastDatesAmount; j++)
            file >> lastDates[j];
        for (auto &date : lastDates) {
            file >> buf;
            for (auto status : buf)
                member.AddPostLight(date, string(1, status));
        }
        member.SetPostsAmount(postsAmount);
        member.SetPostsDatesAmount(postsDatesAmount);
        data[member.GetShortName()] = move(member);
    }

    file.close();
}

void Database::WriteDatabaseToFiles(bool dismission, const string &shortNameDismissed)
{
    for (auto &memberMetaPair : data)
        memberMetaPair.second.GetPostsAmount();
    ofstream file(fileName);

    file << "Members amount: " << data.size() << endl << endl << endl;
    for (auto &memberMetaPair : data) {
        auto shortName = memberMetaPair.first;
        auto &member = memberMetaPair.second;
        file << shortName << '\t' << member.GetRole() << ' ' << member.GetRubric()
             << ' ' << member.GetFrequency() << ' ' << member.GetStartDate() << endl;
        file << "Total posts amount: " << member.GetPostsAmount() << '\t'
             << "Posts dates amount: " << member.GetPostsDatesAmount() << '\t'
             << "Last dates amount: " << member.GetLastDatesAmount() << endl << endl;
        member.PrintPostsLight(file);
        file << endl << endl;

        if (member.ChangedDeepInfo()) {
            ofstream memberFile(GetPath() + shortName + ".md");
            memberFile << shortName << '\t' << member.GetRole() << ' ' << member.GetRubric()
                       << ' ' << member.GetFrequency() << ' ' << member.GetStartDate() << '\\' << endl;
            member.PrintSpecificInfo(memberFile, dismission);
            if (dismission & shortName == shortNameDismissed) {
                PrintPostsAmounts(shortName, memberFile);
                memberFile << "\\" << endl;
            }
            memberFile << "Total posts amount: " << member.GetPostsAmount() << '\t'
                       << "Posts dates amount: " << member.GetPostsDatesAmount() << '\\' << endl;
            member.PrintPosts(memberFile);
            memberFile.close();
            if (dismission & shortName == shortNameDismissed) {
                string oldName = GetPath() + shortName + ".md",
                       newName = GetPath() + shortName + "_dismissed.md";
                rename(oldName.c_str(), newName.c_str());
            }
        }
    }

    file.close();
}

void Database::PrintMembers(bool moreInfo)
{
    cout << endl << "Current members list:" << endl;
    int counter = 0;
    for (auto &memberMetaPair : data)
        communicator->PrintMember(memberMetaPair.second, counter++, moreInfo);
}

void Database::PrintPostsAmounts(const string &shortName, ostream &os)
{
    auto &member = data.at(shortName);
    int postsAmount = member.GetPostsAmount();
    int anticipatedPostsAmount = member.GetAnticipatedPostsAmount();
    string actualPosts =  "Actual posts amount: ";
    os << actualPosts << postsAmount;
    Communication::Tabulator(actualPosts + to_string(postsAmount), 24);
    os << "\tAnticipated posts amount: " << anticipatedPostsAmount;
    if (anticipatedPostsAmount - postsAmount > 1 & !member.OnVacation())
        os << "\t Lag: " << anticipatedPostsAmount - postsAmount;
    if (member.OnVacation())
        os << "\t On vacation";
    os << endl;
}

int Database::GetPostsAmount(const string &shortName, const string &date)
{
    return data.at(shortName).GetPostsAmountAtDate(date);
}

string Database::CollectMemberName(bool moreInfo)
{
    PrintMembers(moreInfo);
    string shortName = Communication::ChooseElem(data);
    if (shortName.empty())
        return shortName;
    cout << endl << "Here are " << shortName << "'s posts: " << endl;
    data.at(shortName).PrintPosts(cout);
    return shortName;
}

bool Database::ContainsMember(const string &shortName) { return bool(data.count(shortName)); }

void Database::AddMember()
{
    PrintMembers();
    cout << endl << "NB: Input without spaces!" << endl;
    cout << endl << "Input member short name: " << endl;
    string shortName = communicator->CollectNewMemberName();
    if (shortName.empty())
        return;
    cout << endl << "Input " << shortName << "'s role: " << endl;
    auto roles = getRoles();
    communicator->PrintRoles(roles);
    string role = Communication::ChooseElem(roles, false);
    cout << endl << "Input " << shortName << "'s rubric: " << endl;
    auto rubrics = getRubrics();
    communicator->PrintRubrics(rubrics);
    string rubric = Communication::ChooseElem(rubrics, false);
    cout << endl << "Input " << shortName << "'s frequency (days needed for 1 post, must be positive integer): " << endl;
    int frequency;
    cin >> frequency;
    string startDate = Date::CollectDate();
    Member member(move(shortName), move(role), move(rubric), frequency, move(startDate), GetPath());
    member.ForceDeepInfoUpdate();
    data[member.GetShortName()] = move(member);
    cout << endl << "New member is successfully added!" << endl;
    WriteDatabaseToFiles();
}

void Database::AddPost()
{
    PostInfo postInfo = communicator->CollectPostInfo(false);
    if (postInfo.Correct) {
        auto &member = data.at(postInfo.ShortName);
        member.AddPost(postInfo.Date, postInfo.Link);
        member.ForceDeepInfoUpdate();
    }
    WriteDatabaseToFiles();
}

void Database::LearnAboutMembers()
{
    string shortName = CollectMemberName(true);
    if (shortName.empty())
        return;
    auto &member = data.at(shortName);
    member.ReadSpecificInfo(GetPath());
    member.PrintInfo();
    member.PrintSpecificInfo(cout);
    PrintPostsAmounts(shortName);
}

void Database::LearnAboutRoles()
{
    auto roles = getRoles();
    communicator->PrintRoles(roles);

    string chosenRole = Communication::ChooseElem(roles);
    if (chosenRole.empty())
        return;
    int counter = 0;
    for (auto &shortName : roles[chosenRole])
        communicator->PrintMember(data.at(shortName), counter++, true);
}

void Database::LearnAboutRubrics()
{
    cout << endl << "Would u like to get rubrics info considering vacations?" << endl;
    auto consideringVacations = bool(Communication::AskForDecision());
    auto rubrics = getRubrics(consideringVacations);
    communicator->PrintRubrics(rubrics);

    string chosenRubric = Communication::ChooseElem(rubrics);
    if (chosenRubric.empty())
        return;
    int counter = 0;
    for (auto &memberMetaPair : data)
        if (memberMetaPair.second.GetRubric() == chosenRubric)
            communicator->PrintMember(memberMetaPair.second, counter++, true);
}

void Database::DeletePost()
{
    PostInfo postInfo = communicator->CollectPostInfo(true);
    if (postInfo.Correct)
        data.at(postInfo.ShortName).DeletePost(postInfo.Date, postInfo.Index);
    WriteDatabaseToFiles();
}

void Database::DeleteMember()
{
    string shortName = CollectMemberName();
    if (shortName.empty())
        return;
    if (data.count(shortName)) {
        auto &member = data.at(shortName);
        if (member.OnVacation())
            member.EndVacation();
        member.ForceDeepInfoUpdate();
        WriteDatabaseToFiles(true, shortName);
        data.erase(shortName);
    } else
        cout << endl << "There is no such member already!" << endl;
    WriteDatabaseToFiles();
}

vector<PostInfo> Database::GetPostsWithStatus(char status)
{
    size_t counter = 0;
    vector<PostInfo> posts;
    cout << endl;
    for (auto &memberMetaPair : data) {
        vector<PostInfo> postsWithSomeStatus = memberMetaPair.second.GetPostsWithStatus(status);
        for (auto &post : postsWithSomeStatus) {
            cout << counter++ << ". " << post.ShortName << ' ' << post.Date << ' ' << post.Link << endl;
            posts.emplace_back(post);
        }
    }
    cout << endl;
    return posts;
}

void Database::ApprovePost()
{
    auto posts = GetPostsWithStatus('-');

    cout << "Input number of the post u are approving or -1 to avoid approving: " << endl;
    size_t counter;
    cin >> counter;

    if (counter == -1)
        return;

    data.at(posts.at(counter).ShortName).SetPostStatus(posts.at(counter).Date, posts.at(counter).Index, '+');

    WriteDatabaseToFiles();
}

void Database::RejectPost()
{
    auto posts = GetPostsWithStatus('-');

    cout << "Input number of the post u are rejecting or -1 to avoid rejecting: " << endl;
    size_t counter;
    cin >> counter;

    if (counter == -1)
        return;

    data.at(posts.at(counter).ShortName).SetPostStatus(posts.at(counter).Date, posts.at(counter).Index, '!');

    WriteDatabaseToFiles();
}

void Database::UpdateDeepInfo()
{
    for (auto &member : data)
        member.second.ForceDeepInfoUpdate();
    WriteDatabaseToFiles();
}

void Database::CheckDuplicates()
{
    auto posts = collectAllPosts();

    cout << endl;
    bool duplicates = false;
    for (auto &post : posts.first)
        if (post.second.size() > 1) {
            duplicates = true;
            cout << "Duplicated post record: " << post.first;
            for (auto &member : post.second)
                cout << ' ' << member;
            cout << endl;
        }

    if (posts.first.size() != posts.second && !duplicates)
        cout << endl << "DATABASE HAS PROBLEMS!!! AT LEAST WITH POSTS AMOUNTS!" << endl;

    cout << "Total unique posts amount: " << posts.first.size() << endl;
    if (duplicates)
        cout << "Total        posts amount: " << posts.second << endl;

    if (!duplicates)
        cout << "No duplicates!" << endl;
}

void Database::FindPosts()
{
    auto posts = collectAllPosts();
    string search;

    cout << endl << "NB: Input without spaces!" << endl;
    cout << endl << "What do u search for (input without spaces)?" << endl;
    cin >> search;

    if (posts.first.count(search)) {
        cout << endl << search << " is posted by " << posts.first[search].back() << endl;
        return;
    }

    if (search.find("vk.com/wall") != -1) {
        string finalLink = Communication::ProceedLink(search);
        if (posts.first.count(finalLink)) {
            cout << endl << finalLink << " is posted by " << posts.first[finalLink].back() << endl;
            return;
        }
    }

    cout << endl;
    bool found = false;
    for (auto &post : posts.first)
        if (post.first.find(search) != -1) {
            cout << post.first << " is posted by " << post.second.back() << endl;
            found = true;
        }

    if (!found)
        cout << "Didn't find anything similar :(" << endl;
}

void Database::ChangeMemberShortName()
{
    string oldShortName = CollectMemberName();
    if (oldShortName.empty())
        return;
    cout << endl << "NB: Input without spaces!" << endl;
    cout << endl << "Input new short name for " << oldShortName << endl;
    string newShortName = communicator->CollectNewMemberName(oldShortName);
    if (newShortName.empty())
        return;
    data.at(oldShortName).Rename(newShortName);
    data[newShortName] = move(data[oldShortName]);
    data.erase(oldShortName);
    string oldName = GetPath() + oldShortName + ".md",
            newName = GetPath() + newShortName + ".md";
    rename(oldName.c_str(), newName.c_str());
    WriteDatabaseToFiles();
}

void Database::ChangeMemberRole()
{
    string shortName = CollectMemberName();
    if (shortName.empty())
        return;
    auto &member = data.at(shortName);
    member.PrintInfo();
    cout << endl << "NB: Input without spaces!" << endl;
    cout << endl << "Input new role for " << shortName << endl;
    auto roles = getRoles();
    communicator->PrintRoles(roles);
    string newRole = Communication::ChooseElem(roles, false);
    member.ChangeRole(newRole);
    WriteDatabaseToFiles();
}

void Database::ChangeMemberRubric()
{
    string shortName = CollectMemberName();
    if (shortName.empty())
        return;
    auto &member = data.at(shortName);
    member.PrintInfo();
    cout << endl << "NB: Input without spaces!" << endl;
    cout << endl << "Input new rubric for " << shortName << endl;
    auto rubrics = getRubrics();
    communicator->PrintRubrics(rubrics);
    string newRubric = Communication::ChooseElem(rubrics, false);
    member.ChangeRubric(newRubric);
    WriteDatabaseToFiles();
}

void Database::ChangeMemberFrequency()
{
    string shortName = CollectMemberName();
    if (shortName.empty())
        return;
    auto &member = data.at(shortName);
    member.PrintInfo();
    cout << endl << "NB: Input without spaces!" << endl;
    cout << endl << "Input new frequency for " << shortName << " (days needed for 1 post, must be positive integer): " << endl;
    int newFrequency;
    cin >> newFrequency;
    member.ChangeFrequency(newFrequency);
    WriteDatabaseToFiles();
}

void Database::AddVacation()
{
    string shortName = CollectMemberName();
    if (shortName.empty())
        return;
    auto &member = data.at(shortName);
    member.PrintInfo();
    string start, end;
    cout << endl << "NB: Input without spaces!" << endl;
    cout << endl << "Input vacation start date: " << endl;
    start = Date::CollectDate();
    cout << endl << "Input vacation end date or \"unlimited\": " << endl;
    end = Date::CollectDate(true);
    member.AddVacation(start, end);
    member.ForceDeepInfoUpdate();
    WriteDatabaseToFiles();
}

bool Database::TalkToUser()
{
    if (communicator == nullptr)
        communicator = make_unique<Communication>(Communication(this));
    return communicator->TalkToUser();
}

string Database::GetFileName() const
{
    if (fileName.find('/') == -1)
        return fileName;
    return fileName.substr(fileName.find_last_of('/') + 1);
}

string Database::GetPath() const
{
    if (fileName.find('/') == -1)
        return "";
    return fileName.substr(0, fileName.find_last_of('/') + 1);
}

Database::~Database() {}