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
    std::string id;
    std::string role;
    std::string rubric;
    int frequency;
    Date start;
    std::map<std::string, std::string> rubricSwitches;
    std::map<std::string, std::pair<int, int>> frequencySwitches;
    std::map<std::string, std::string> vacations;
    int postsAmount, postsDatesAmount;
    std::map<std::string, std::vector<TwoStrings>> posts, lightPosts;
    bool changedDeepInfo, loadedDeepInfo;
    std::string path;

public:
    Member();

    Member(std::string &&shortName, std::string &&id, std::string &&role, std::string &&rubric, int frequency, std::string &&start, std::string &&path);

    bool AddPost(const std::string &date, const std::string &link = "", const std::string &status = "");

    bool AddPostLight(const std::string &date, const std::string &status = "");

    void SetPostStatus(const std::string &date, int index, char status);

    void DeletePost(const std::string &date, int index);

    int GetPostsAmountAtDate(const std::string &date);

    std::vector<TwoStrings> GetPostsAtDate(const std::string &date);

    std::vector<TwoStrings> GetAllPosts();

    std::vector<PostInfo> GetPostsWithStatus(char status);

    void PrintPosts(std::ostream &os);

    void PrintPostsLight(std::ostream &os);

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

    void SetPostsAmount(int amount);

    void SetPostsDatesAmount(int amount);

    int GetPostsAmount() const;

    size_t GetPostsDatesAmount() const;

    int GetLastDatesAmount() const;

    int GetVacationLength(const std::string &startDate, const std::string &endDate);

    bool OnVacation();

    void EndVacation();

    int GetAnticipatedPostsAmount();

    void ReadSpecificInfo(const std::string &path);

    void PrintSpecificInfo(std::ostream &os, bool dismission = false) const;

    bool ChangedDeepInfo() const ;
};