#include "parser.hpp"
#include "combinator.hpp"

#include <iostream>
#include <functional>

Parser::Parser(const std::string& code) :
    ParserCombinators(code), m_ast(NodeType::List)
{}

void Parser::parse()
{
    while (!isEOF())
    {
        // parsing single line comments as instructions
        space();
        if (!isEOF())
            comment();
        else
            break;
        space();

        auto n = node();
        if (n)
            m_ast.push_back(n.value());
    }

    for (auto block : m_ast.list())
        std::cout << block << "\n";
}

bool Parser::comment()
{
    if (accept(IsChar('#')))
    {
        while (accept(IsNot(IsChar('\n'))))
            ;
        accept(IsChar('\n'));
        return true;
    }
    return false;
}

std::optional<Node> Parser::node()
{
    std::vector<std::function<std::optional<Node>()>> methods = {
        [this]() -> std::optional<Node> {
            return letMutSet();
        },
        [this]() -> std::optional<Node> {
            return del();
        },
        [this]() -> std::optional<Node> {
            return condition();
        },
        [this]() -> std::optional<Node> {
            return loop();
        },
        [this]() -> std::optional<Node> {
            return import_();
        },
        //[this]() -> std::optional<Node> { return block(); },
        [this]() -> std::optional<Node> {
            return function();
        },
        [this]() -> std::optional<Node> {
            return macro();
        },
    };

    if (!accept(IsChar('(')))   // FIXME can not implement import nor begin if this stays here
                                // TODO make a node parser wrapper, taking charge of the surrounding ()
        return std::nullopt;
    space();

    // save current position in buffer to be able to go back if needed
    auto position = getCount();

    for (std::size_t i = 0, end = methods.size(); i < end; ++i)
    {
        auto result = methods[i]();

        if (result.has_value())
        {
            space();
            if (accept(IsChar(')')))
                return result;
            else
                errorWithNextToken("Missing closing paren after node");
        }
        else
            backtrack(position);
    }

    errorWithNextToken("Couldn't parse node");
    return std::nullopt;  // will never reach
}

std::optional<Node> Parser::letMutSet()
{
    std::string keyword;
    if (!oneOf({"let", "mut", "set"}, &keyword))
        return std::nullopt;

    space();

    std::string symbol;
    if (!name(&symbol))
        errorWithNextToken(keyword + " needs a symbol");

    space();

    auto value = atom();
    if (!value)
        errorWithNextToken("Expected a value");

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(Node(NodeType::Symbol, symbol));
    leaf.push_back(value.value());

    return leaf;
}

std::optional<Node> Parser::del()
{
    std::string keyword;
    if (!oneOf({"del"}, &keyword))
        return std::nullopt;

    space();

    std::string symbol;
    if (!name(&symbol))
        errorWithNextToken(keyword + " needs a symbol");

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(Node(NodeType::Symbol, symbol));

    return leaf;
}

std::optional<Node> Parser::condition()
{
    std::string keyword;
    if (!oneOf({"if"}, &keyword))
        return std::nullopt;

    space();

    auto condition = atom();
    if (!condition)
        errorWithNextToken("If need a valid condition");  // TODO handle nodes

    space();

    auto value_if_true = atom();  // TODO handle nodes
    if (!value_if_true)
        errorWithNextToken("Expected a value");

    space();

    auto value_if_false = atom();  // TODO handle nodes
    if (value_if_false)
        space();

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(condition.value());
    leaf.push_back(value_if_true.value());
    if (value_if_false)
        leaf.push_back(value_if_false.value());

    return leaf;
}

std::optional<Node> Parser::loop()
{
    std::string keyword;
    if (!oneOf({"while"}, &keyword))
        return std::nullopt;

    space();

    auto condition = atom();
    if (!condition)
        errorWithNextToken("While need a valid condition");  // TODO handle nodes

    space();

    auto body = atom();  // TODO handle nodes
    if (!body)
        errorWithNextToken("Expected a value");

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(condition.value());
    leaf.push_back(body.value());

    return leaf;
}

std::optional<Node> Parser::import_()
{
    // TODO import parser
    // (import folder.bidule.machin :a :b :c)
    // (import a)
    // (import a:*)

    std::string keyword;
    if (!oneOf({"import"}, &keyword))
        return std::nullopt;

    space();

    std::string symbol;
    if (!name(&symbol))
        errorWithNextToken(keyword + " needs a symbol");

    if (accept(IsChar('(')))
    {
        space();



        expect(IsChar(')'));
        space();
    }

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(Node(NodeType::Symbol, symbol));

    return leaf;
}

std::optional<Node> Parser::block()
{
    return std::nullopt;
}

std::optional<Node> Parser::function()
{
    std::string keyword;
    if (!oneOf({"fun"}, &keyword))
        return std::nullopt;

    space();

    expect(IsChar('('));
    space();

    Node args(NodeType::List);

    while (true)
    {
        std::string symbol;
        if (!name(&symbol))
            break;
        else
        {
            space();
            args.push_back(Node(NodeType::Symbol, symbol));
        }
    }

    expect(IsChar(')'));
    space();

    auto value = atom();
    if (!value)  // TODO handle nodes
        errorWithNextToken("Expected a value");

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(args);
    leaf.push_back(value.value());

    return leaf;
}

std::optional<Node> Parser::macro()
{
    std::string keyword;
    if (!oneOf({"macro"}, &keyword))
        return std::nullopt;

    space();

    std::string symbol;
    if (!name(&symbol))
        errorWithNextToken(keyword + " needs a symbol");

    space();

    std::optional<Node> args;

    if (accept(IsChar('(')))
    {
        space();
        args = Node(NodeType::List);

        while (true)
        {
            std::string symbol;
            if (!name(&symbol))
                break;
            else
            {
                space();
                args->push_back(Node(NodeType::Symbol, symbol));
            }
        }

        expect(IsChar(')'));
        space();
    }


    auto value = atom();
    if (!value)  // TODO handle nodes
        errorWithNextToken("Expected a value");

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(Node(NodeType::Symbol, symbol));
    if (args.has_value())
        leaf.push_back(args.value());
    leaf.push_back(value.value());

    return leaf;
}

std::optional<Node> Parser::atom()
{
    std::vector<std::function<std::optional<Node>()>> parsers = {
        // numbers
        [this]() -> std::optional<Node> {
            std::string res;
            if (signedNumber(&res))
                return Node(std::stoi(res));  // FIXME stoi?
            return std::nullopt;
        },
        // strings
        [this]() -> std::optional<Node> {
            std::string res;
            if (accept(IsChar('"')))
            {
                while (accept(IsNot(IsChar('"')), &res))
                    ;
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

void Parser::errorWithNextToken(const std::string& message)
{
    std::string next_token;
    anyUntil(IsInlineSpace, &next_token);
    error(message, next_token);
}
