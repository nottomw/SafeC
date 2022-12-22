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

int main(int argc, char **argv)
{
    po::options_description desc("All SafeC transpiler options:");
    desc.add_options()                                                                      //
        ("help,h", "print help")                                                            //
        ("file,f", po::value<std::vector<std::string>>(), "SafeC file(s) to be transpiled") //
        ("output,o", po::value<std::string>(), "C output files directory")                  //
        ("disable,d", po::value<std::vector<std::string>>(), "disable SafeC options");

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

    const auto outputDirectory = bfs::path(vm["output"].as<std::string>());
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
            safec::log("\t--> %").arg(it);
        }
    }

    if (vm.count("file") > 0)
    {
        safec::Parser parser{safec::SemanticsFactory::get()};

        safec::log("Parsing files:");
        auto &filesToParse = vm["file"].as<std::vector<std::string>>();
        for (const auto &it : filesToParse)
        {
            safec::log("\t--> %").arg(it);

            parser.parse(it);

            // TODO: generate output file in outputDirectory
        }
    }

    return 0;
}
