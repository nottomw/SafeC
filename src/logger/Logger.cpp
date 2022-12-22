#include "Logger.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace safec
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
#define LOGGER_TERM_COLOR_MAGENTA "\033[01;35m"

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
        case Color::Magenta:
            return LOGGER_TERM_COLOR_MAGENTA;
        default:
            break;
    }

    assert(nullptr == "invalid logger color");
    return LOGGER_TERM_COLOR_NC;
}

LogHelper::LogHelper(const char *const formatString, logger::Properties &&props)
    : mFormatString{formatString}
    , mProperties{std::move(props)}
{
}

LogHelper &LogHelper::arg(const std::string &a)
{
    assert(mArgsLeft > 0U);

    const uint32_t argIndex = mArgsOffsets.size() - mArgsLeft;
    const uint32_t argOffset = mArgsOffsets[argIndex];

    // align saved % offsets
    const size_t argStrSize = a.size() - 1;
    std::for_each(mArgsOffsets.begin() + argIndex, //
                  mArgsOffsets.end(),
                  [argStrSize](size_t &off) { off += argStrSize; });

    mFormatString.replace(argOffset, 1, a);

    mArgsLeft--;

    logIfAllArgsProvided();
    return *this;
}

LogHelper &LogHelper::arg(const char a)
{
    std::string str(1, a);
    return arg(str);
}

LogHelper &LogHelper::arg(const int32_t a)
{
    std::string argStr = std::to_string(a);
    return arg(argStr);
}

LogHelper &LogHelper::arg(const uint32_t a)
{
    std::string argStr = std::to_string(a);
    return arg(argStr);
}

LogHelper &LogHelper::arg(const char *const a)
{
    return arg(std::string(a));
}

void LogHelper::logIfAllArgsProvided()
{
    if (mArgsLeft == 0U)
    {
        std::cout << colorToTermColor(mProperties.getColor()) //
                  << mFormatString                            //
                  << colorToTermColor(Color::NoColor);

        if (mProperties.getNewLine() == logger::NewLine::Yes)
        {
            std::cout << '\n';
        }
    }
}

LogHelper log(const char *const formatString, logger::Properties &&props)
{
    assert(formatString != nullptr);

    uint32_t argsLeft = 0U;
    std::vector<size_t> argsOffsets;

    uint32_t idx = 0U;
    while (formatString[idx] != '\0')
    {
        if (formatString[idx] == '%')
        {
            if (formatString[idx + 1U] == '%')
            {
                // double percent, needs to be escaped
                idx++;
            }
            else
            {
                argsOffsets.push_back(idx);
                argsLeft++;
            }
        }

        idx++;
    }

    if (argsLeft == 0U)
    {
        std::cout << colorToTermColor(props.getColor()) //
                  << formatString                       //
                  << colorToTermColor(Color::NoColor)   //
                  << '\n';
    }
    else
    {
        LogHelper helper{formatString, std::move(props)};
        helper.mArgsLeft = argsLeft;
        helper.mArgsOffsets = std::move(argsOffsets);

        return helper;
    }

    return LogHelper{formatString, std::move(props)};
}

logger::Properties::Properties(const Color color, const NewLine nl)
    : mNewLine{nl}
    , mColor{color}
{
}

logger::Properties::Properties(const NewLine nl)
    : mNewLine{nl}
    , mColor{Color::NoColor}
{
}

logger::NewLine logger::Properties::getNewLine() const
{
    return mNewLine;
}

Color logger::Properties::getColor() const
{
    return mColor;
}

} // namespace safec
