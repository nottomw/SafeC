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

    void setGenerate(const bool g)
    {
        mGenerate = g;
    }

    bool getGenerate() const
    {
        return mGenerate;
    }

    void setDisplayAstMod(const bool m)
    {
        mDisplayAstMod = m;
    }

    bool getDisplayAstMod() const
    {
        return mDisplayAstMod;
    }

private:
    Config()
        : mDisplayAst{false}
        , mDisplayParserInfo{false}
        , mNoColor{false}
        , mDisplayCoverage{false}
        , mGenerate{true}
        , mDisplayAstMod{false}
    {
    }

    bool mDisplayAst;
    bool mDisplayParserInfo;
    bool mNoColor;
    bool mDisplayCoverage;
    bool mGenerate;
    bool mDisplayAstMod;
};

} // namespace safec
