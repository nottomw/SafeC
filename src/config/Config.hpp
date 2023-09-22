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
    void setDisplayParserInfo(const bool display)
    {
        mDisplayParserInfo = display;
    }

    bool getDisplayAst() const
    {
        return mDisplayAst;
    }
    bool getDisplayParserInfo() const
    {
        return mDisplayParserInfo;
    }

private:
    Config()
        : mDisplayAst{false}
        , mDisplayParserInfo{false} {};

    bool mDisplayAst;
    bool mDisplayParserInfo;
};

} // namespace safec
