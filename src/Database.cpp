#include "Database.hpp"

#include <cmath>
#include <fstream>

#include "Math.hpp"

using namespace std;

void Database::tabulator(const string &str, int threshold)
{
    if (str.size() < threshold)
        cout << '\t';
}

template <typename IterableContainer>
string Database::chooseElem(const IterableContainer &elems, bool exitPossible)
{
    string elemName;
    bool correct = true;
    do {
        if (!correct)
            cout << endl << "No such element! Try again.";
        bool digits = true;
        cout << endl << "Input element name (u can also input according number)" << endl;
        if (exitPossible)
            cout << endl << "Or input -1 to return =)" << endl;
        cin >> elemName;
        if (exitPossible && elemName == "-1")
            return "";
        for (auto c : elemName)
            if (!isdigit(c)) {
                digits = false;
                break;
            }
        if (digits) {
            int counter = 0;
            for (auto &elem : elems)
                if (stoi(elemName) == counter++) {
                    elemName = elem.first;
                    break;
                }
        }
    } while (exitPossible && !(correct = bool(elems.count(elemName))));
    return elemName;
}

void Database::printMember(Member &member, int counter, bool moreInfo)
{
    cout << counter << ". " << member.GetShortName() << '\t';
    tabulator(to_string(counter) + ". " + member.GetShortName(), 16);
    if (moreInfo) {
        cout << "Rubric: "  << member.GetRubric() << '\t';
        tabulator(member.GetRubric(), 8);
        cout << "Frequency: " << member.GetFrequency() << '\t';
        printPostsAmounts(member.GetShortName());
    }
    else
        cout << endl;
}

void Database::printMembers(bool moreInfo)
{
    cout << endl << "Current members list:" << endl;
    int counter = 0;
    for (auto &elem : data)
        printMember(elem.second, counter++, moreInfo);
}

void Database::printPostsAmounts(const string &shortName, ostream &os)
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

string Database::collectMemberName(bool moreInfo)
{
    printMembers(moreInfo);
    string shortName = chooseElem(data, moreInfo);
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

void Database::addMember()
{
    printMembers();
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
    string role = chooseElem(roles);
    cout << endl << "Input member rubric: " << endl;
    auto rubrics = getRubrics();
    printRubrics(rubrics);
    string rubric = chooseElem(rubrics);
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

void Database::addPost()
{
    PostInfo postInfo = collectPostInfo(false);
    if (postInfo.Correct) {
        auto &member = data[postInfo.ShortName];
        member.AddPost(postInfo.Date, postInfo.Link);
        member.ForceDeepInfoUpdate();
    }
    WriteDatabaseToFiles();
}

void Database::approvePost()
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

void Database::learnAboutMembers()
{
    string shortName = collectMemberName(true);
    if (shortName.empty())
        return;
    data[shortName].PrintInfo();
    data[shortName].PrintSpecificInfo(cout);
    printPostsAmounts(shortName);
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

void Database::learnAboutRoles()
{
    auto roles = getRoles();
    printRoles(roles);

    string chosenRole = chooseElem(roles, true);
    if (chosenRole.empty())
        return;
    int counter = 0;
    for (auto &shortName : roles[chosenRole])
        printMember(data[shortName], counter++, true);
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
        tabulator(to_string(counter) + ". " + rubric.first, 7);
        cout << "Editors amount: " << rubric.second.size()
             << "\tOverall frequency (days needed for 1 post): " << statistics[counter]
             << " (" << ceil(statistics[counter]) << ')' << endl;
        counter++;
    }
}

void Database::learnAboutRubrics()
{
    cout << endl << "Would u like to get rubrics info considering vacations?" << endl;
    auto consideringVacations = bool(AskForDecision());
    auto rubrics = getRubrics(consideringVacations);
    printRubrics(rubrics);

    string chosenRubric = chooseElem(rubrics, true);
    if (chosenRubric.empty())
        return;
    int counter = 0;
    for (auto &elem : data)
        if (elem.second.GetRubric() == chosenRubric)
            printMember(elem.second, counter++, true);
}

void Database::deletePost()
{
    PostInfo postInfo = collectPostInfo(true);
    if (postInfo.Correct)
        data[postInfo.ShortName].DeletePost(postInfo.Date, postInfo.Index);
    WriteDatabaseToFiles();
}

void Database::deleteMember()
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

void Database::changeMemberShortName()
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

void Database::changeMemberRole()
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

void Database::changeMemberRubric()
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

void Database::changeMemberFrequency()
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

void Database::addVacation()
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

void Database::changeMemberData()
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

void Database::checkDuplicates()
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

void Database::findPosts()
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
        string finalLink = ProceedLink(search);
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

Database::Database(string &&fileName) : fileName(fileName)
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
                printPostsAmounts(elem.first, memberFile);
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

bool Database::TalkToUser()
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
            for (auto &member : data)
                member.second.ForceDeepInfoUpdate();
            break;
        case 10:
            checkDuplicates();
            break;
        case 11:
            findPosts();
            break;
        default:
            cout << endl << "You made some mistake :(" << endl;
            break;
    }

    cout << endl << "Would u like to do something else?" << endl;
    return bool(AskForDecision());
}

int Database::AskForDecision()
{
    int decision;
    cout << endl << "Input 1 for Yes and 0 for No" << endl;
    cin >> decision;
    return decision;
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

void Database::FileProblems()
{
    cout << endl << "Something is wrong with your database file, it should be near your executable & be named "
         << GetFileName() << " :)" << endl << "Proper file is being created now!" << endl;
}

string Database::ProceedLink(const std::string &link)
{
    string finalLink = link;
    if (link.find("http") == -1)
        finalLink = "https://" + link;
    if (finalLink.find("mu_marveluniverse") == -1)
        finalLink = finalLink.substr(0, 15) + "mu_marveluniverse?w=" + finalLink.substr(15);
    return finalLink;
}

Database::~Database() { WriteDatabaseToFiles(); }