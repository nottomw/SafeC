#pragma once

namespace safec
{

class Config
{
public:
    ~Config() = default;

    static Config &getInstance()
    {
        static Config cfg;
        return cfg;
    }

    void setDisplayAst(const bool display)
    {
        mDisplayAst = display;
    }

    bool getDisplayAst() const
    {
        return mDisplayAst;
    }

    void setDisplayParserInfo(const bool display)
    {
        mDisplayParserInfo = display;
    }

    bool getDisplayParserInfo() const
    {
        return mDisplayParserInfo;
    }

    void setNoColor(const bool noColor)
    {
        mNoColor = noColor;
    }

    bool getNoColor() const
    {
        return mNoColor;
    }

    void setDisplayCoverage(const bool disp)
    {
        mDisplayCoverage = disp;
    }

    bool getDisplayCoverage() const
    {
        return mDisplayCoverage;
    }

private:
    Config()
        : mDisplayAst{false}
        , mDisplayParserInfo{false} {};

    bool mDisplayAst;
    bool mDisplayParserInfo;
    bool mNoColor;
    bool mDisplayCoverage;
};

} // namespace safec
