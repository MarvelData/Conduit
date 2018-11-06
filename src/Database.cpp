#include "Database.hpp"

#include <cmath>
#include <fstream>

#include "Communication.hpp"
#include "Math.hpp"

using namespace std;

string Database::collectMemberName(bool moreInfo)
{
    PrintMembers(moreInfo);
    string shortName = Communication::ChooseElem(data, moreInfo);
    if (shortName.empty())
        return shortName;
    cout << endl << "Here are his/her posts: " << endl;
    data.at(shortName).PrintPosts(cout);
    return shortName;
}

PostInfo Database::collectPostInfo(bool deleting)
{
    string shortName, date, link;
    int index = -1;
    shortName = collectMemberName();
    date = Date::CollectDate();
    if (deleting) {
        int amount = data[shortName].GetPostsAmountAtDate(date);
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

map<string, vector<string>> Database::getRoles()
{
    map<string, vector<string>> roles;
    for (auto &elem : data)
        roles[elem.second.GetRole()].emplace_back(elem.first);
    return roles;
}

void Database::printRoles(const map<string, vector<string>> &roles)
{
    cout << endl << "Current roles list:" << endl;
    int counter = 0;
    for (auto &role : roles)
        cout << counter << ". " << role.first << endl;
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

void Database::printRubrics(const map<string, vector<int>> &rubrics)
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
        Communication::Tabulator(to_string(counter) + ". " + rubric.first, 7);
        cout << "Editors amount: " << rubric.second.size()
             << "\tOverall frequency (days needed for 1 post): " << statistics[counter]
             << " (" << ceil(statistics[counter]) << ')' << endl;
        counter++;
    }
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
    auto &member = data[shortName];
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

void Database::AddMember()
{
    PrintMembers();
    string shortName, startDate;
    int frequency;
    cout << endl << "NB: Input without spaces!" << endl;
    cout << endl << "Input member short name: " << endl;
    cin >> shortName;
    if (shortName.empty()) {
        cout << endl << "You can't add empty short name :(" << endl;
        return;
    }
    cout << endl << "Input member role: " << endl;
    auto roles = getRoles();
    printRoles(roles);
    string role = Communication::ChooseElem(roles);
    cout << endl << "Input member rubric: " << endl;
    auto rubrics = getRubrics();
    printRubrics(rubrics);
    string rubric = Communication::ChooseElem(rubrics);
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

void Database::AddPost()
{
    PostInfo postInfo = collectPostInfo(false);
    if (postInfo.Correct) {
        auto &member = data[postInfo.ShortName];
        member.AddPost(postInfo.Date, postInfo.Link);
        member.ForceDeepInfoUpdate();
    }
    WriteDatabaseToFiles();
}

void Database::LearnAboutMembers()
{
    string shortName = collectMemberName(true);
    if (shortName.empty())
        return;
    data[shortName].PrintInfo();
    data[shortName].PrintSpecificInfo(cout);
    PrintPostsAmounts(shortName);
}

void Database::LearnAboutRoles()
{
    auto roles = getRoles();
    printRoles(roles);

    string chosenRole = Communication::ChooseElem(roles, true);
    if (chosenRole.empty())
        return;
    int counter = 0;
    for (auto &shortName : roles[chosenRole])
        communicator->PrintMember(data[shortName], counter++, true);
}

void Database::LearnAboutRubrics()
{
    cout << endl << "Would u like to get rubrics info considering vacations?" << endl;
    auto consideringVacations = bool(Communication::AskForDecision());
    auto rubrics = getRubrics(consideringVacations);
    printRubrics(rubrics);

    string chosenRubric = Communication::ChooseElem(rubrics, true);
    if (chosenRubric.empty())
        return;
    int counter = 0;
    for (auto &elem : data)
        if (elem.second.GetRubric() == chosenRubric)
            communicator->PrintMember(elem.second, counter++, true);
}

void Database::DeletePost()
{
    PostInfo postInfo = collectPostInfo(true);
    if (postInfo.Correct)
        data[postInfo.ShortName].DeletePost(postInfo.Date, postInfo.Index);
    WriteDatabaseToFiles();
}

void Database::DeleteMember()
{
    string shortName = collectMemberName();
    if (data.count(shortName)) {
        auto &member = data[shortName];
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
    string oldName = GetPath() + oldShortName + ".md",
            newName = GetPath() + newShortName + ".md";
    rename(oldName.c_str(), newName.c_str());
    WriteDatabaseToFiles();
}

void Database::ChangeMemberRole()
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

void Database::ChangeMemberRubric()
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

void Database::ChangeMemberFrequency()
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

void Database::AddVacation()
{
    string shortName = collectMemberName();
    auto &member = data[shortName];
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
    if (!communicator)
        communicator = new Communication(this);
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

Database::~Database()
{
    delete communicator;
    WriteDatabaseToFiles();
}