#include <cstdio>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
	po::options_description desc("Allowed options");
	desc.add_options() //
		("help", "print help");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		printf("help...");
		return 0;
	}

	printf("nothing yet...\n");
	return 0;
}
