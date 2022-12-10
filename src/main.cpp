#include <iostream>
#include <vector>
#include <string>
#include "parser/Parser.hpp"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

// TODO: add logger...

int main(int argc, char **argv)
{
	po::options_description desc("All SafeC transpiler options:");
	desc.add_options()																		//
		("help,h", "print help")															//
		("file,f", po::value<std::vector<std::string>>(), "SafeC file(s) to be transpiled") //
		("disable,d", po::value<std::vector<std::string>>(), "disable SafeC options");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help") > 0)
	{
		std::cout << desc;

		return 0;
	}

	if (vm.count("disable") > 0)
	{
		std::cout << "Disabled features: \n";
		auto &disabledFeatures = vm["disable"].as<std::vector<std::string>>();
		for (const auto &it : disabledFeatures)
		{
			std::cout << " - " << it << std::endl;
		}
	}

	if (vm.count("file") > 0)
	{
		safec::Parser p;

		std::cout << "Parsing files:\n";
		auto &filesToParse = vm["file"].as<std::vector<std::string>>();
		for (const auto &it : filesToParse)
		{
			std::cout << " - " << it << std::endl;
			p.parse(it);
		}
	}

	std::cout << "not functional yet...\n";

	return 0;
}
