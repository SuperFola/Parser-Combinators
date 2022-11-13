#include "parser.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Expected one argument: filename" << std::endl;
        return 1;
    }

    std::string filename(argv[1]);

    return 0;
}