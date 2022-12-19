#include "Logger.hpp"

#include <cassert>
#include <iostream>

namespace safec::log
{

#define LOGGER_TERM_COLOR_NC "\033[0m"
#define LOGGER_TERM_COLOR_YELLOW "\033[00;33m"
#define LOGGER_TERM_COLOR_RED "\033[00;31m"
#define LOGGER_TERM_COLOR_GREEN "\033[00;32m"
#define LOGGER_TERM_COLOR_LRED "\033[01;31m"
#define LOGGER_TERM_COLOR_LGREEN "\033[01;32m"
#define LOGGER_TERM_COLOR_LYELLOW "\033[01;33m"
#define LOGGER_TERM_COLOR_LBLUE "\033[01;34m"
#define LOGGER_TERM_COLOR_LPURPLE "\033[01;35m"
#define LOGGER_TERM_COLOR_LCYAN "\033[01;36m"

static constexpr const char *colorToTermColor(Color c)
{
    switch (c)
    {
        case Color::NoColor:
            return LOGGER_TERM_COLOR_NC;
        case Color::Yellow:
            return LOGGER_TERM_COLOR_YELLOW;
        case Color::Red:
            return LOGGER_TERM_COLOR_RED;
        case Color::Green:
            return LOGGER_TERM_COLOR_GREEN;
        case Color::LightRed:
            return LOGGER_TERM_COLOR_LRED;
        case Color::LightGreen:
            return LOGGER_TERM_COLOR_LGREEN;
        case Color::LightYellow:
            return LOGGER_TERM_COLOR_LYELLOW;
        case Color::LightBlue:
            return LOGGER_TERM_COLOR_LBLUE;
        case Color::LightPurple:
            return LOGGER_TERM_COLOR_LPURPLE;
        case Color::LightCyan:
            return LOGGER_TERM_COLOR_LCYAN;
        default:
            break;
    }

    assert(nullptr == "invalid logger color");
    return LOGGER_TERM_COLOR_NC;
}

LogHelper::LogHelper(const char *const formatString, Color color)
    : mFormatString{formatString}
    , mColor{color}
{
}

LogHelper &LogHelper::arg(const uint32_t a)
{
    assert(mArgsLeft > 0U);

    size_t pos = mFormatString.find(mFormatChar);
    assert(pos != std::string::npos);

    mFormatString.replace(pos, 1, std::to_string(a));

    mArgsLeft--;

    logIfAllArgsProvided();
    return *this;
}

LogHelper &LogHelper::arg(const char *const a)
{
    assert(mArgsLeft > 0U);

    size_t pos = mFormatString.find(mFormatChar);
    assert(pos != std::string::npos);

    mFormatString.replace(pos, 1, a);

    mArgsLeft--;

    logIfAllArgsProvided();
    return *this;
}

LogHelper &LogHelper::arg(const std::string &a)
{
    assert(mArgsLeft > 0U);

    size_t pos = mFormatString.find(mFormatChar);
    assert(pos != std::string::npos);

    mFormatString.replace(pos, 1, a);

    mArgsLeft--;

    logIfAllArgsProvided();
    return *this;
}

LogHelper &LogHelper::noNewLine()
{
    mNoNewLine = true;
    return *this;
}

void LogHelper::logIfAllArgsProvided()
{
    if (mArgsLeft == 0U)
    {
        std::cout << colorToTermColor(mColor) //
                  << mFormatString            //
                  << colorToTermColor(Color::NoColor);

        if (mNoNewLine == false)
        {
            std::cout << '\n';
        }
    }
}

LogHelper log(const char *const formatString, Color color)
{
    assert(formatString != nullptr);

    LogHelper helper{formatString, color};

    // TODO: escape for %%

    const char *p = formatString;
    while (*p != '\0')
    {
        if (*p == '%')
        {
            helper.mArgsLeft++;
        }

        p++;
    }

    if (helper.mArgsLeft == 0U)
    {
        std::cout << colorToTermColor(color)          //
                  << formatString                     //
                  << colorToTermColor(Color::NoColor) //
                  << '\n';
    }

    return helper;
}

void syntaxReport(const uint32_t stringIndex, const char *const name, const Color color)
{
    safec::log::log("@ % at % @", color).noNewLine().arg(name).arg(stringIndex);
}

} // namespace safec::log
