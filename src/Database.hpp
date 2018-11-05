#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Member.hpp"

class Database
{
    std::string fileName;
    std::map<std::string, Member> data;

    void tabulator(const std::string &str, int threshold);

    template <typename IterableContainer>
    std::string chooseElem(const IterableContainer &elems, bool exitPossible = false);

    void printMember(Member &member, int counter = 0, bool moreInfo = false);

    void printMembers(bool moreInfo = false);

    void printPostsAmounts(const std::string &shortName, std::ostream &os = std::cout);

    std::string collectMemberName(bool moreInfo = false);

    PostInfo collectPostInfo(bool deleting);

    void addMember();

    void addPost();

    void approvePost();

    void learnAboutMembers();

    std::map<std::string, std::vector<std::string>> getRoles();

    void printRoles(const std::map<std::string, std::vector<std::string>> &roles);

    void learnAboutRoles();

    std::map<std::string, std::vector<int>> getRubrics(bool consideringVacations = false);

    void printRubrics(const std::map<std::string, std::vector<int>> &rubrics);

    void learnAboutRubrics();

    void deletePost();

    void deleteMember();

    void changeMemberShortName();

    void changeMemberRole();

    void changeMemberRubric();

    void changeMemberFrequency();

    void addVacation();

    void changeMemberData();

    std::pair<std::map<std::string, std::vector<std::string>>, int> collectAllPosts();

    void checkDuplicates();

    void findPosts();

public:
    explicit Database(std::string &&fileName);

    void AddMember(std::string shortName, std::string role, std::string rubric, int frequency, std::string startDate);

    void AddPost(const std::string &shortName, const std::string &date, std::string &link);

    void WriteDatabaseToFiles(bool dismission = false);

    bool TalkToUser();

    int AskForDecision();

    std::string GetFileName() const;

    std::string GetPath() const;
    
    void FileProblems();

    static std::string ProceedLink(const std::string &link);

    ~Database();
};