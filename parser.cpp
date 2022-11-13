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
    auto position = getCount();

    if (!accept(IsChar('(')))
    {
        std::cout << "Expected an opening paren to create a new node" << std::endl;
        return std::nullopt;
    }
    space();

    std::vector<std::function<std::optional<Node>()>> methods = {
        std::bind(&Parser::letMutSet, *this),
        std::bind(&Parser::del, *this),
        std::bind(&Parser::condition, *this),
        std::bind(&Parser::loop, *this),
        std::bind(&Parser::import_, *this),
        std::bind(&Parser::block, *this),
        std::bind(&Parser::function, *this),
        std::bind(&Parser::macro, *this),
    };

    for (auto method : methods)
    {
        if (auto result = method())
        {
            space();
            if (accept(IsChar(')')))
                return result;
            else
            {
                error("Missing closing paren after node", ")");
                return std::nullopt;
            }
        }
        else
            backtrack(position);
    }

    return std::nullopt;
}

std::optional<Node> Parser::letMutSet()
{
    // eat the trailing white space
    space();

    std::string keyword = "";
    if (!name(&keyword))
        return std::nullopt;
    if (keyword != "let" && keyword != "mut" && keyword != "set")
        return std::nullopt;

    space();

    std::string symbol = "";
    if (!name(&symbol))
        return std::nullopt;

    space();

    auto value = atom();
    if (!value)
    {
        // TODO throw exception
        std::cout << "Expected a value" << std::endl;
        return std::nullopt;
    }

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(Node(NodeType::Symbol, symbol));
    leaf.push_back(value.value());

    return leaf;
}

std::optional<Node> Parser::del()
{
    return std::nullopt;
}

std::optional<Node> Parser::condition()
{
    return std::nullopt;
}

std::optional<Node> Parser::loop()
{
    return std::nullopt;
}

std::optional<Node> Parser::import_()
{
    return std::nullopt;
}

std::optional<Node> Parser::block()
{
    return std::nullopt;
}

std::optional<Node> Parser::function()
{
    return std::nullopt;
}

std::optional<Node> Parser::macro()
{
    return std::nullopt;
}

std::optional<Node> Parser::atom()
{
    std::vector<std::function<std::optional<Node>()>> parsers = {
        // numbers
        [this]() -> std::optional<Node> {
            std::string res;
            if (signedNumber(&res))
                return Node(std::stoi(res));
            return std::nullopt;
        },
        // strings
        [this]() -> std::optional<Node> {
            std::string res;
            if (accept(IsChar('"')))
            {
                while (accept(IsNot(IsChar('"')), &res));
                expect(IsChar('"'));

                return Node(NodeType::String, res);
            }
            return std::nullopt;
        },
        // true/false/nil
        [this]() -> std::optional<Node> {
            std::string res;
            if (!name(&res))
                return std::nullopt;
            if (res == "false" || res == "true" || res == "nil")
                return Node(NodeType::Symbol, res);
            return std::nullopt;
        }
    };

    auto pos = getCount();

    for (auto parser : parsers)
    {
        if (auto result = parser())
            return result;
        else
            backtrack(pos);
    }

    return std::nullopt;
}
