#include "Logger.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace safec
{

// move to some utils?
#define STRINGIFY(x) #x

// clang-format off
#define COLORS_ENUMERATE_IN_SWITCH_CASE(color_name, color_value) \
        case Color::color_name: \
            return "\033[" STRINGIFY(color_value) "m"; \
            break;
// clang-format on

static constexpr const char *colorToTermColor(Color c)
{
    switch (c)
    {
        COLORS_ENUMERATE(COLORS_ENUMERATE_IN_SWITCH_CASE);

        default:
            break;
    }

    assert(nullptr == "invalid logger color");
    return "";
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

LogHelper &LogHelper::arg(const size_t a)
{
    return arg(std::to_string(a));
}

LogHelper &LogHelper::arg(const char *const a)
{
    return arg(std::string(a));
}

LogHelper &LogHelper::arg(const std::string_view a)
{
    return arg(std::string(a));
}

void LogHelper::logIfAllArgsProvided()
{
    if (mArgsLeft == 0U)
    {
        const char *const bgColor =                      //
            (mProperties.getColorBg() == Color::NoColor) //
                ? ("")
                : (colorToTermColor(mProperties.getColorBg()));

        std::cout << colorToTermColor(mProperties.getColor()) //
                  << bgColor                                  //
                  << mFormatString                            //
                  << colorToTermColor(Color::NoColor);

        if (mProperties.getNewLine() == NewLine::Yes)
        {
            std::cout << '\n';
        }

        fflush(stdout);
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
        const char *const bgColor =                //
            (props.getColorBg() == Color::NoColor) //
                ? ("")
                : (colorToTermColor(props.getColorBg()));

        std::cout << colorToTermColor(props.getColor()) //
                  << bgColor                            //
                  << formatString                       //
                  << colorToTermColor(Color::NoColor);  //

        if (props.getNewLine() == NewLine::Yes)
        {
            std::cout << '\n';
        }

        fflush(stdout);
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
    , mBgColor{Color::NoColor}
{
}

logger::Properties::Properties( //
    const Color color,
    const Color bgColor,
    const NewLine nl)
    : mNewLine{nl}
    , mColor{color}
    , mBgColor{bgColor}
{
}

logger::Properties::Properties(const NewLine nl)
    : mNewLine{nl}
    , mColor{Color::NoColor}
    , mBgColor{Color::NoColor}
{
}

logger::Properties::Properties()
    : mNewLine{NewLine::Yes}
    , mColor{Color::NoColor}
    , mBgColor{Color::NoColor}
{
}

NewLine logger::Properties::getNewLine() const
{
    return mNewLine;
}

Color logger::Properties::getColor() const
{
    return mColor;
}

Color logger::Properties::getColorBg() const
{
    return mBgColor;
}

} // namespace safec
