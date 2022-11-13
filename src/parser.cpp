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
        space();
        comment();
        space();

        auto n = node();
        if (n)
            m_ast.push_back(n.value());
    }

    std::cout << m_ast << std::endl;
}

bool Parser::comment()
{
    if (accept(IsChar('#')))
    {
        while (accept(IsNot(IsChar('\n'))));
        accept(IsChar('\n'));
        return true;
    }
    return false;
}

std::optional<Node> Parser::node()
{
    // save current position in buffer to be able to go back if needed
    auto position = getCount();

    if (!accept(IsChar('(')))
        return std::nullopt;
    space();

    std::vector<std::function<std::optional<Node>()>> methods = {
        [this]() -> std::optional<Node> { return letMutSet(); },
        [this]() -> std::optional<Node> { return del(); },
        [this]() -> std::optional<Node> { return condition(); },
        [this]() -> std::optional<Node> { return loop(); },
        [this]() -> std::optional<Node> { return import_(); },
        [this]() -> std::optional<Node> { return block(); },
        [this]() -> std::optional<Node> { return function(); },
        [this]() -> std::optional<Node> { return macro(); },
    };

    for (auto method : methods)
    {
        if (auto result = method(); result.has_value())
        {
            space();
            if (accept(IsChar(')')))
                return result;
            else
                error("Missing closing paren after node", ")");
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
        error(keyword + " needs a symbol", keyword);

    space();

    auto value = atom();
    if (!value)
    {
        error("Expected a value", symbol);
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
        auto result = parser();
        if (result.has_value())
            return result;
        else
            backtrack(pos);
    }

    return std::nullopt;
}
