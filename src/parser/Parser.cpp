#include "Parser.hpp"
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

namespace safec
{

    namespace bfs = ::boost::filesystem;
    namespace bio = ::boost::iostreams;

    void Parser::parse(const std::string &filePath)
    {
        const bfs::path filePathBoost{filePath};
        if (bfs::exists(filePathBoost) == false)
        {
            throw std::runtime_error("file not found");
        }

        if (bfs::is_directory(filePathBoost) == true)
        {
            std::cout << "parser: got directory\n";
            // TODO: for each .sc file in directory...
        }

        if (bfs::is_regular_file(filePathBoost) == true)
        {
            std::cout << "parser: got file\n";
            bio::mapped_file_source source{filePathBoost};

            const std::string_view sourceView{source.data(), source.size()};

            std::cout << sourceView << std::endl;

            // TODO: boost spirit/qi needed ?
        }
    }

}
