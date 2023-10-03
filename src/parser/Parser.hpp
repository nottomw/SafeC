#pragma once

#include <any>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

namespace safec
{

class Semantics;
class SemNodeTranslationUnit;

class Parser final
{
public:
    Parser(Semantics &sem);
    ~Parser() = default;

    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    Parser &operator=(const Parser &) = delete;
    Parser &operator=(Parser &&) = delete;

    size_t parse(const std::string &path);
    void displayAst() const;
    void displayCoverage() const;

    std::shared_ptr<SemNodeTranslationUnit> getAst() const;

private:
    size_t parseFile(const std::filesystem::path &path);

    Semantics &mSemantics;
    std::filesystem::path mCurrentlyParsedFile;
};

} // namespace safec
