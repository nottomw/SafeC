#include "config/Config.hpp"
#include "generator/Generator.hpp"
#include "logger/Logger.hpp"
#include "parser/Parser.hpp"
#include "parser/semantics/SemanticsFactory.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

// TODO: maybe use std::filesystem instead of boost::filesystem
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
        ("debug", "debug mode - display all possible info");

    // TODO: add option to print AST
    // TODO: add option to echo lexed/parsed file
    // TODO: add option to switch parser to C99/GNU/other

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

    if (vm.count("debug") != 0)
    {
        safec::Config::getInstance().setDisplayAst(true);
        safec::Config::getInstance().setDisplayParserInfo(true);
    }

    const auto outputDirectory = bfs::absolute(vm["output"].as<std::string>());
    if (bfs::is_directory(outputDirectory) == false)
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

            //            const auto outputFileName = bfs::path{it}.filename().replace_extension("c");
            //            const auto outputFileFullPath = (outputDirectory / outputFileName).normalize();

            //            safec::log("Generating C file: '%'", outputFileFullPath.c_str());

            //            safec::Generator generator{parser};
            //            generator.generate(outputFileFullPath);
        }
    }

    return 0;
}
