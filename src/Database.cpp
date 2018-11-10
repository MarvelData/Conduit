#include "Database.hpp"

#include <fstream>

#include "Communication.hpp"

using namespace std;

map<string, vector<string>> Database::getRoles()
{
    map<string, vector<string>> roles;
    for (auto &elem : data)
        roles[elem.second.GetRole()].emplace_back(elem.first);
    return roles;
}

map<string, vector<int>> Database::getRubrics(bool consideringVacations)
{
    map<string, vector<int>> rubrics;
    if (consideringVacations)
        for (auto &elem : data) {
            elem.second.ReadSpecificInfo(GetPath());
            if (!elem.second.OnVacation())
                rubrics[elem.second.GetRubric()].emplace_back(elem.second.GetFrequency());
        }
    else
        for (auto &elem : data)
            rubrics[elem.second.GetRubric()].emplace_back(elem.second.GetFrequency());
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
                member.AddPost(date, link, buf);
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

void Database::WriteDatabaseToFiles(bool dismission)
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
            elem.second.ReadSpecificInfo(GetPath());
            ofstream memberFile(GetPath() + elem.first + ".md");
            memberFile << elem.first << '\t' << elem.second.GetRole() << ' ' << elem.second.GetRubric()
                       << ' ' << elem.second.GetFrequency() << ' ' << elem.second.GetStartDate() << '\\' << endl;
            elem.second.PrintSpecificInfo(memberFile, dismission);
            if (dismission) {
                PrintPostsAmounts(elem.first, memberFile);
                memberFile << "\\" << endl;
            }
            memberFile << "Total posts amount: " << elem.second.GetPostsAmount() << '\t'
                       << "Posts dates amount: " << elem.second.GetPostsDatesAmount() << '\\' << endl;
            elem.second.PrintPosts(memberFile);
            memberFile.close();
            if (dismission) {
                string oldName = GetPath() + elem.first + ".md",
                       newName = GetPath() + elem.first + "_dismissed.md";
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
    for (auto &elem : data)
        communicator->PrintMember(elem.second, counter++, moreInfo);
}

void Database::PrintPostsAmounts(const string &shortName, ostream &os)
{
    auto &member = data.at(shortName);
    member.ReadSpecificInfo(GetPath());
    int postsAmount = member.GetPostsAmount();
    int anticipatedPostsAmount = member.GetAnticipatedPostsAmount();
    os << "Actual posts amount: " << postsAmount;
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
    Member member(move(shortName), move(role), move(rubric), frequency, move(startDate));
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
    for (auto &elem : data)
        if (elem.second.GetRubric() == chosenRubric)
            communicator->PrintMember(elem.second, counter++, true);
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
        member.ReadSpecificInfo(GetPath());
        if (member.OnVacation())
            member.EndVacation();
        member.ForceDeepInfoUpdate();
        WriteDatabaseToFiles(true);
        data.erase(shortName);
    } else
        cout << endl << "There is no such member already!" << endl;
    WriteDatabaseToFiles();
}

void Database::ApprovePost()
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

void Database::UpdateDeepInfo()
{
    for (auto &member : data)
        member.second.ForceDeepInfoUpdate();
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
        cout << "Didn't found anything similar :(" << endl;
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
        communicator = unique_ptr<Communication>(new Communication(this));
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