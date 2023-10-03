#include "config/Config.hpp"
#include "generator/Generator.hpp"
#include "logger/Logger.hpp"
#include "parser/Parser.hpp"
#include "semantics/SemanticsFactory.hpp"

#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace po = boost::program_options;
namespace fs = std::filesystem;

// TODO: some proper functional/unit tests

int main(int argc, char **argv)
{
    po::options_description desc("All SafeC transpiler options:");
    desc.add_options()                                                                               //
        ("help,h", "print help")                                                                     //
        ("file,f", po::value<std::vector<std::string>>(), "SafeC file(s) to be transpiled")          //
        ("output,o", po::value<std::string>(), "C output files directory")                           //
        ("disable,d", po::value<std::vector<std::string>>(), "disable SafeC options { defer, ... }") //
        ("astdump,a", "dump AST")                                                                    //
        ("parserdump,p", "dump parser info")                                                         //
        ("nocolor,n", "do not add color to logs")                                                    //
        ("coverage,c", "display coverage info")                                                      //
        ("generate", "generate the output C file - now for debug purposes")                          //
        ("debug", "debug mode - display all possible info");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") > 0)
    {
        std::cout << desc << std::endl;
        return 0;
    }

    if (vm.count("output") == 0)
    {
        safec::log("missing output file(s) directory");
        return -1;
    }

    if (vm.count("astdump") != 0)
    {
        safec::Config::getInstance().setDisplayAst(true);
    }

    if (vm.count("parserdump") != 0)
    {
        safec::Config::getInstance().setDisplayParserInfo(true);
    }

    if (vm.count("nocolor") != 0)
    {
        safec::Config::getInstance().setNoColor(true);
    }

    if (vm.count("coverage") != 0)
    {
        safec::Config::getInstance().setDisplayCoverage(true);
    }

    // eventually should generate by default, but for now hidden behind an option
    if (vm.count("generate") != 0)
    {
        safec::Config::getInstance().setGenerate(true);
    }
    else
    {
        safec::Config::getInstance().setGenerate(false);
    }

    if (vm.count("debug") != 0)
    {
        safec::Config::getInstance().setDisplayAst(true);
        safec::Config::getInstance().setDisplayParserInfo(true);
        safec::Config::getInstance().setDisplayCoverage(true);
        safec::Config::getInstance().setGenerate(true);
    }

    const auto outputDirectory = fs::absolute(vm["output"].as<std::string>());
    if (fs::is_directory(outputDirectory) == false)
    {
        safec::log("provided 'output' parameter does not point to a directory");
        return -1;
    }

    if (vm.count("disable") > 0)
    {
        safec::log("Disabled features:");
        auto &disabledFeatures = vm["disable"].as<std::vector<std::string>>();
        for (const auto &it : disabledFeatures)
        {
            safec::log("\t--> %", it);
        }
    }

    if (vm.count("file") > 0)
    {
        safec::Parser parser{safec::SemanticsFactory::get()};

        auto &filesToParse = vm["file"].as<std::vector<std::string>>();
        for (const auto &it : filesToParse)
        {
            safec::log("Parsing file: '%'...", it);
            const size_t charCount = parser.parse(it);
            safec::log("Parsing done, characters count %\n", charCount);

            if (safec::Config::getInstance().getDisplayAst() == true)
            {
                parser.displayAst();
            }

            if (safec::Config::getInstance().getDisplayCoverage() == true)
            {
                parser.displayCoverage();
            }

            if (safec::Config::getInstance().getGenerate())
            {
                const auto outputFileName = fs::path{it}.filename().replace_extension("c");
                const auto outputFileFullPath = std::filesystem::weakly_canonical(outputDirectory / outputFileName);

                safec::log("Generating C file: '%'", outputFileFullPath.c_str());
                safec::Generator generator;
                generator.generate(parser.getAst(), outputFileFullPath);
            }
        }
    }

    return 0;
}
