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
        space();
        while (!isEOF() && comment())
            space();
        if (isEOF())
            break;

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
            return wrapped(&Parser::letMutSet, '(', ')');
        },
        [this]() -> std::optional<Node> {
            return wrapped(&Parser::del, '(', ')');
        },
        [this]() -> std::optional<Node> {
            return wrapped(&Parser::condition, '(', ')');
        },
        [this]() -> std::optional<Node> {
            return wrapped(&Parser::loop, '(', ')');
        },
        [this]() -> std::optional<Node> {
            return import_();
        },
        //[this]() -> std::optional<Node> { return block(); },
        [this]() -> std::optional<Node> {
            return wrapped(&Parser::function, '(', ')');
        },
        [this]() -> std::optional<Node> {
            return wrapped(&Parser::macro, '(', ')');
        },
        [this]() -> std::optional<Node> {
            return functionCall();
        },
    };

    // save current position in buffer to be able to go back if needed
    auto position = getCount();

    for (std::size_t i = 0, end = methods.size(); i < end; ++i)
    {
        auto result = methods[i]();

        if (result.has_value())
            return result;
        else
            backtrack(position);
    }

    return std::nullopt;  // will never reach
}

std::optional<Node> Parser::letMutSet()
{
    std::string keyword;
    if (!oneOf({ "let", "mut", "set" }, &keyword))
        return std::nullopt;
    space();

    std::string symbol;
    if (!name(&symbol))
        errorWithNextToken(keyword + " needs a symbol");
    space();

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(Node(NodeType::Symbol, symbol));

    if (auto value = nodeOrValue(); value.has_value())
        leaf.push_back(value.value());
    else
        errorWithNextToken("Expected a value");

    return leaf;
}

std::optional<Node> Parser::del()
{
    std::string keyword;
    if (!oneOf({ "del" }, &keyword))
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
    if (!oneOf({ "if" }, &keyword))
        return std::nullopt;

    space();

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));

    if (auto condition = nodeOrValue(); condition.has_value())
        leaf.push_back(condition.value());
    else
        errorWithNextToken("If need a valid condition");

    space();

    if (auto value_if_true = nodeOrValue(); value_if_true.has_value())
        leaf.push_back(value_if_true.value());
    else
        errorWithNextToken("Expected a value");

    space();

    if (auto value_if_false = nodeOrValue(); value_if_false.has_value())
    {
        leaf.push_back(value_if_false.value());
        space();
    }

    return leaf;
}

std::optional<Node> Parser::loop()
{
    std::string keyword;
    if (!oneOf({ "while" }, &keyword))
        return std::nullopt;

    space();

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));

    if (auto condition = nodeOrValue(); condition.has_value())
        leaf.push_back(condition.value());
    else
        errorWithNextToken("While need a valid condition");

    space();

    if (auto body = nodeOrValue(); body.has_value())
        leaf.push_back(body.value());
    else
        errorWithNextToken("Expected a value");

    return leaf;
}

std::optional<Node> Parser::import_()
{
    // TODO import parser
    // (import folder.bidule.machin :a :b :c)
    // (import a)
    // (import a:*)

    std::string keyword;
    if (!oneOf({ "import" }, &keyword))
        return std::nullopt;

    space();

    std::string symbol;
    if (!name(&symbol))
        errorWithNextToken(keyword + " needs a symbol");

    if (accept(IsChar('(')))
    {
        space();
        // TODO argument list of import?
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
    if (!oneOf({ "fun" }, &keyword))
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

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(args);

    if (auto value = nodeOrValue(); value.has_value())
        leaf.push_back(value.value());
    else
        errorWithNextToken("Expected a value");

    return leaf;
}

std::optional<Node> Parser::macro()
{
    std::string keyword;
    if (!oneOf({ "macro" }, &keyword))
        return std::nullopt;
    space();

    std::string symbol;
    if (!name(&symbol))
        errorWithNextToken(keyword + " needs a symbol");
    space();

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));
    leaf.push_back(Node(NodeType::Symbol, symbol));

    if (accept(IsChar('(')))
    {
        space();
        Node args = Node(NodeType::List);

        while (true)
        {
            std::string arg_name;
            if (!name(&arg_name))
                break;
            else
            {
                space();
                args.push_back(Node(NodeType::Symbol, arg_name));
            }
        }

        expect(IsChar(')'));
        space();

        leaf.push_back(args);
    }

    if (auto value = nodeOrValue(); value.has_value())
        leaf.push_back(value.value());
    else
        errorWithNextToken("Expected a value");

    return leaf;
}

std::optional<Node> Parser::functionCall()
{
    if (!accept(IsChar('(')))
        return std::nullopt;
    space();

    std::string symbol;
    if (!name(&symbol))
        return std::nullopt;
    space();

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Symbol, symbol));

    while (true)
    {
        if (auto arg = nodeOrValue(); arg.has_value())
        {
            space();
            leaf.push_back(arg.value());
        }
        else
            break;
    }

    space();
    expect(IsChar(')'));
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
        // true/false/nil/...
        [this]() -> std::optional<Node> {
            std::string res;
            if (!name(&res))
                return std::nullopt;
            return Node(NodeType::Symbol, res);
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

std::optional<Node> Parser::nodeOrValue()
{
    if (auto value = atom(); value.has_value())
        return value;
    else if (auto sub_node = node(); sub_node.has_value())
        return sub_node;

    return std::nullopt;
}

std::optional<Node> Parser::wrapped(std::optional<Node> (Parser::*parser)(), char prefix, char suffix)
{
    if (!accept(IsChar(prefix)))
        return std::nullopt;
    space();

    std::optional<Node> node = (this->*parser)();

    if (node)
    {
        space();
        if (accept(IsChar(suffix)))
            return node;
        else
            errorWithNextToken("Missing '" + std::string(1, suffix) + "' after node");
    }

    return std::nullopt;
}

void Parser::errorWithNextToken(const std::string& message)
{
    std::string next_token;
    anyUntil(IsInlineSpace, &next_token);
    error(message, next_token);
}
