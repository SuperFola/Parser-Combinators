#include "parser.hpp"
#include "combinator.hpp"

#include <iostream>

Parser::Parser(const std::string& code) :
    m_ast(NodeType::List), ParserCombinators(code)
{}

void Parser::parse()
{
    while (!isEOF())
    {
        // parsing single line comments as instructions
        while (comment())
            endOfLine();

        auto n = node();
        if (!n)
        {
            // TODO error handling
            std::cout << "error occured when parsing" << std::endl;
            break;
        }
        else
        {
            m_ast.push_back(n.value());
        }
    }
}

bool Parser::comment()
{
    inlineSpace();
    if (accept(IsChar('#')))
    {
        while (accept(IsNot(IsChar('\n'))));
        return true;
    }
    return false;
}

std::optional<Node> Parser::node()
{
    // save current position in buffer to be able to go back if needed
    auto current = getCount();

    return std::nullopt;
}