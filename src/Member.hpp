#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Date.hpp"
#include "PostInfo.hpp"

typedef std::pair<std::string, std::string> TwoStrings;

class Member {
    std::string shortName;
    std::string role;
    std::string rubric;
    int frequency;
    Date start;
    std::map<std::string, std::string> rubricSwitches;
    std::map<std::string, std::pair<int, int>> frequencySwitches;
    std::map<std::string, std::string> vacations;
    int postsAmount;
    std::map<std::string, std::vector<TwoStrings>> posts;
    bool changedDeepInfo, loadedDeepInfo;

public:
    Member();

    Member(std::string &&shortName, std::string &&role, std::string &&rubric, int frequency, std::string &&start);

    bool AddPost(const std::string &date, const std::string &link, const std::string &status = "");

    void ApprovePost(const std::string &date, int index);

    void DeletePost(const std::string &date, int index);

    int GetPostsAmountAtDate(const std::string &date) const;

    std::vector<TwoStrings> GetPostsAtDate(const std::string &date) const;

    std::vector<PostInfo> GetNotApprovedPosts() const;

    void PrintPosts(std::ostream &os) const;

    void PrintInfo() const;

    void Rename(const std::string &newShortName);

    void ChangeRole(const std::string &newRole);

    void ChangeRubric(const std::string &newRubric);

    void ChangeFrequency(int newFrequency);

    void AddVacation(const std::string &startDate, const std::string &endDate);

    void ForceDeepInfoUpdate();

    std::string GetShortName() const;

    std::string GetRole() const;

    std::string GetRubric() const;

    int GetFrequency() const;

    Date GetStartDate() const;

    int GetPostsAmount() const;

    size_t GetPostsDatesAmount() const;

    int GetVacationLength(const std::string &startDate, const std::string &endDate) const;

    bool OnVacation() const;

    void EndVacation();

    int GetAnticipatedPostsAmount() const;

    void ReadSpecificInfo(const std::string &path);

    void PrintSpecificInfo(std::ostream &os, bool dismission = false) const;

    bool ChangedDeepInfo() const ;
};