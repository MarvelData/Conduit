#pragma once

#include <string>

class Date
{
    std::string date;
    int day, month, year;
    size_t daySymbols = 2, monthSymbols = 2, yearSymbols = 4, dotsAmount = 2;

public:
    explicit Date(const std::string &date);

    int Since(const Date &oldDate);

    std::string GetAsString() const;

    static int GetDaysInMonth(int year, int month);

    static void DateProblems();

    static bool CheckDate(const std::string &date);

    static std::string CollectDate(bool unlimitedPossibility = false);

    static std::string TimeToDate(const time_t &time);

    static std::string Now();

    static std::string Yesterday();

    friend std::ostream& operator<<(std::ostream& os, const Date& date);
};