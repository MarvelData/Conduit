#include "Date.hpp"

#include <chrono>
#include <ctime>
#include <iostream>

using namespace std;

Date::Date(const string &date) : date(date)
{
    if (CheckDate(date)) {
        year = stoi(date.substr(0, yearSymbols));
        month = stoi(date.substr(yearSymbols + 1, monthSymbols));
        day = stoi(date.substr(yearSymbols + monthSymbols + dotsAmount, daySymbols));
    } else {
        day = month = year = -1;
        DateProblems();
    }
}

int Date::Since(const Date &oldDate)
{
    if (oldDate.year > year) {
        cout << endl << "Wrong dates order!" << endl;
        return -1;
    }
    int diff = 0;
    if (year == oldDate.year) {
        if (oldDate.month > month) {
            cout << endl << "Wrong dates order!" << endl;
            return -1;
        }
        if (month == oldDate.month) {
            if (oldDate.day > day) {
                cout << endl << "Wrong dates order!" << endl;
                return -1;
            }
            return day - oldDate.day;
        }
        for (int i = oldDate.month + 1; i < month; i++)
            diff += GetDaysInMonth(year, i);
    } else {
        for (int i = oldDate.year + 1; i < year; i++)
            for (int j = 1; j <= 12; j++)
                diff += GetDaysInMonth(i, j);
        for (int i = oldDate.month + 1; i <= 12; i++)
            diff += GetDaysInMonth(oldDate.year, i);
        for (int i = 1; i < month; i++)
            diff += GetDaysInMonth(year, i);

    }
    return diff + GetDaysInMonth(oldDate.year, oldDate.month) - oldDate.day + day;
}

string Date::GetAsString() const { return date; }

int Date::GetDaysInMonth(int year, int month)
{
    switch (month)
    {
        case 1:
            return 31;
        case 2:
            if (year % 4)
                return 28;
            return 29;
        case 3:
            return 31;
        case 4:
            return 30;
        case 5:
            return 31;
        case 6:
            return 30;
        case 7:
            return 31;
        case 8:
            return 31;
        case 9:
            return 30;
        case 10:
            return 31;
        case 11:
            return 30;
        case 12:
            return 31;
        default:
            cout << endl << "There is some mistake with date management:(" << endl;
            return -1;
    }
}

void Date::DateProblems()
{
    cout << endl << "Something is wrong with your date, it should look like YYYY.MM.DD :)" << endl;
}

bool Date::CheckDate(const string &date)
{
    if (date.length() != 10)
        return false;
    if (date[4] != '.' || date[7] != '.')
        return false;
    for (auto c : date)
        if (c != '.' && !isdigit(c))
            return false;
    if (stoi(date.substr(8, 2)) < 1 || stoi(date.substr(8, 2)) > 31)
        return false;
    if (stoi(date.substr(5, 2)) < 1 || stoi(date.substr(5, 2)) > 12)
        return false;

    return true;
}

string Date::CollectDate(bool unlimitedPossibility)
{
    string date;
    bool correct = true;
    do {
        if (!correct)
            DateProblems();
        cout << endl << "Input date in format YYYY.MM.DD (to use current date, just input 0, yesterday : -1): " << endl;
        cin >> date;
        if (unlimitedPossibility && date == "unlimited")
            return date;
        if (date == "0")
            return Now();
        if (date == "-1")
            return Yesterday();
    } while (!(correct = CheckDate(date)));
    return date;
}

string Date::TimeToDate(const time_t &time)
{
    string date;
    date = to_string(localtime(&time)->tm_year + 1900) + '.';
    if (localtime(&time)->tm_mon + 1 < 10)
        date += '0';
    date += to_string(localtime(&time)->tm_mon + 1) + '.';
    if (localtime(&time)->tm_mday < 10)
        date += '0';
    date += to_string(localtime(&time)->tm_mday);
    return date;
}

string Date::Now() { return TimeToDate(chrono::system_clock::to_time_t(chrono::system_clock::now())); }

string Date::Yesterday() { return TimeToDate(chrono::system_clock::to_time_t(chrono::system_clock::now()) - 86400); }

ostream& operator<<(ostream& os, const Date& date)
{
    os << date.date;
    return os;
}