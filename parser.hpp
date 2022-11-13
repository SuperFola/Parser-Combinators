#ifndef PARSER_HPP
#define PARSER_HPP

#include "combinator.hpp"

#include <string>

class Parser : public ParserCombinators
{
public:
    Parser(const std::string& code);

    void parse();

private:
};

#endif
