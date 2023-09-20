#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace safec
{

// clang-format off
#define COLORS_ENUMERATE(entry) \
    entry(NoColor,  0) \
    entry(Black,    30) \
    entry(Red,      31) \
    entry(Green,    32) \
    entry(Yellow,   33) \
    entry(Blue,     34) \
    entry(Magenta,  35) \
    entry(Cyan,     36) \
    entry(White,    37) \
    entry(Gray,     90) \
    entry(BgBlack,  40) \
    entry(BgRed,    41) \
    entry(BgGreen,  42) \
    entry(BgYellow, 43) \
    entry(BgBlue,   44) \
    entry(BgMagenta,45) \
    entry(BgCyan,   46) \
    entry(BgWhite,  47)
// clang-format on

#define COLORS_ENUMERATE_IN_ENUM(color_name, color_value) color_name,

enum class Color
{
    COLORS_ENUMERATE(COLORS_ENUMERATE_IN_ENUM)
};

enum class NewLine
{
    Yes,
    No
};

namespace logger
{

class Properties
{
public:
    Properties(const Color color, const NewLine nl = NewLine::Yes);
    Properties(const Color color, const Color bgColor, const NewLine nl = NewLine::Yes);
    Properties(const NewLine nl);
    Properties();

    NewLine getNewLine() const;
    Color getColor() const;
    Color getColorBg() const;

private:
    const NewLine mNewLine;
    const Color mColor;
    const Color mBgColor;
};

} // namespace logger

class LogHelper
{
public:
    LogHelper(const char *const formatString, logger::Properties &&props);

    LogHelper &arg(const std::string &a);
    LogHelper &arg(const char a);
    LogHelper &arg(const int32_t a);
    LogHelper &arg(const uint32_t a);
    LogHelper &arg(const size_t a);
    LogHelper &arg(const char *const a);
    LogHelper &arg(const std::string_view a);

private:
    static constexpr char mFormatChar = '%';
    uint32_t mArgsLeft{0U};
    std::string mFormatString;
    logger::Properties mProperties;
    std::vector<size_t> mArgsOffsets;

    void logIfAllArgsProvided();

    friend LogHelper log(const char *const, logger::Properties &&);
};

// TODO: add option to color only parts of string?

// example: log("text % % %").arg(5).arg(3.14).arg("text");
LogHelper log(const char *const formatString, logger::Properties &&props = logger::Properties{});

} // namespace safec
