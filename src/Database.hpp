#pragma once

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <vector>

#include "Member.hpp"

class Communication;

class Database
{
    std::string fileName;
    std::map<std::string, Member> data;
    std::unique_ptr<Communication> communicator;

    std::map<std::string, std::vector<std::string>> getRoles();

    std::map<std::string, std::vector<int>> getRubrics(bool consideringVacations = false);

    std::pair<std::map<std::string, std::vector<std::string>>, int> collectAllPosts();

public:
    explicit Database(std::string &&fileName);

    void WriteDatabaseToFiles(bool dismission = false, const std::string &shortNameDismissed = std::string());

    void PrintMembers(bool moreInfo = false);

    void PrintPostsAmounts(const std::string &shortName, std::ostream &os = std::cout);

    int GetPostsAmount(const std::string &shortName, const std::string &date);

    std::string CollectMemberName(bool moreInfo = false);

    bool ContainsMember(const std::string &shortName);

    void AddMember();

    void AddMember(std::string shortName, std::string role, std::string rubric, int frequency, std::string startDate);

    void AddPost();

    void AddPost(const std::string &shortName, const std::string &date, std::string &link);

    void LearnAboutMembers();

    void LearnAboutRoles();

    void LearnAboutRubrics();

    void DeletePost();

    void DeleteMember();

    void ApprovePost();

    void UpdateDeepInfo();

    void CheckDuplicates();

    void FindPosts();

    void ChangeMemberShortName();

    void ChangeMemberRole();

    void ChangeMemberRubric();

    void ChangeMemberFrequency();

    void AddVacation();

    bool TalkToUser();

    std::string GetFileName() const;

    std::string GetPath() const;

    Database(const Database &) = delete;
    Database(Database &&) = delete;
    virtual ~Database();
};