#include "parser.hpp"

Parser::Parser(const std::string& code) :
    ParserCombinators(code)
{}

void Parser::parse()
{
    while (!isEOF())
    {
        // TODO
    }
}