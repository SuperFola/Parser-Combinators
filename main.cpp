#include "parser.hpp"

#include <iostream>
#include <string>
#include <fstream>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Expected one argument: filename" << std::endl;
        return 1;
    }

    std::string filename(argv[1]);

    std::ifstream stream(filename);
    if (!stream.is_open())
        std::cout << "Failed to open " << filename << '\n';
    else
    {
        std::string code((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        Parser parser(code);
        parser.parse();
    }

    return 0;
}