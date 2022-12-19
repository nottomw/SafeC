#pragma once

#include <cstdint>
#include <string>

namespace safec
{

enum class Color
{
    NoColor,
    Yellow,
    Red,
    Green,
    LightRed,
    LightGreen,
    LightYellow,
    LightBlue,
    LightPurple,
    LightCyan
};

namespace logger
{

enum class NewLine
{
    Yes,
    No
};

class Properties
{
public:
    Properties(const Color color = Color::NoColor, const NewLine nl = NewLine::Yes);
    Properties(const NewLine nl);

    NewLine getNewLine() const;
    Color getColor() const;

private:
    const NewLine mNewLine;
    const Color mColor;
};

} // namespace logger

class LogHelper
{
public:
    LogHelper(const char *const formatString, logger::Properties &&props);

    LogHelper &arg(const uint32_t a);
    LogHelper &arg(const char *const a);
    LogHelper &arg(const std::string &a);

private:
    static constexpr char mFormatChar = '%';
    uint32_t mArgsLeft{0U};
    std::string mFormatString;
    logger::Properties mProperties;

    void logIfAllArgsProvided();

    friend LogHelper log(const char *const, logger::Properties &&);
};

// example: log("text % % %").arg(5).arg(3.14).arg("text");
LogHelper log(const char *const formatString, logger::Properties &&props = logger::Properties{});

} // namespace safec
