#include "parser.hpp"

#include <iostream>
#include <functional>

Parser::Parser(const std::string& code, bool debug) :
    BaseParser(code), m_ast(NodeType::List), m_debug(debug)
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

    if (m_debug)
    {
        for (auto block : m_ast.list())
            std::cout << block << "\n";
    }
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
        [this]() -> std::optional<Node> {
            return block();
        },
        [this]() -> std::optional<Node> {
            return wrapped(&Parser::function, '(', ')');
        },
        [this]() -> std::optional<Node> {
            return wrapped(&Parser::macro, '(', ')');
        },
        [this]() -> std::optional<Node> {
            return functionCall();
        },
        [this]() -> std::optional<Node> {
            return list();
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
    // (import foo.bar.egg :a :b :c)
    // (import a)
    // (import a:*)

    if (!accept(IsChar('(')))
        return std::nullopt;
    space();

    std::string keyword;
    if (!oneOf({ "import" }, &keyword))
        return std::nullopt;
    space();

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, keyword));

    std::string package;
    if (!name(&package))
        errorWithNextToken("Import expected a package name");

    Node packageNode(NodeType::List);
    packageNode.push_back(Node(NodeType::String, package));
    Node symbols(NodeType::List);

    while (true)
    {
        // parsing package folder.foo.bar.yes
        if (accept(IsChar('.')))
        {
            std::string path;
            if (!name(&path))
                errorWithNextToken("Package name expected after '.'");
            else
                packageNode.push_back(Node(NodeType::String, path));
        }
        else if (accept(IsChar(':')) && accept(IsChar('*')))  // parsing :*
        {
            if (symbols.list().size() != 0)
            {
                backtrack(getCount() - 2);
                error("Star pattern can not follow a symbol to import", ":*");
            }

            space();
            expect(IsChar(')'));

            leaf.push_back(packageNode);
            leaf.push_back(Node(NodeType::Symbol, "*"));

            return leaf;
        }
        else if (space())  // parsing potential :a :b :c
        {
            if (accept(IsChar(':')))
            {
                std::string symbol;
                if (!name(&symbol))
                    errorWithNextToken("Expected a valid symbol to import");

                symbols.push_back(Node(NodeType::Symbol, symbol));
            }
        }
        else
            break;
    }

    leaf.push_back(packageNode);
    leaf.push_back(symbols);

    space();
    expect(IsChar(')'));
    return leaf;
}

std::optional<Node> Parser::block()
{
    bool alt_syntax = false;
    if (accept(IsChar('(')))
    {
        space();
        if (!oneOf({ "begin" }))
            return std::nullopt;
    }
    else if (accept(IsChar('{')))
        alt_syntax = true;
    else
        return std::nullopt;
    space();

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Keyword, "begin"));

    while (true)
    {
        if (auto value = nodeOrValue(); value.has_value())
        {
            leaf.push_back(value.value());
            space();
        }
        else
            break;
    }

    space();
    expect(IsChar(!alt_syntax ? ')' : '}'));
    return leaf;
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
    bool has_captures = false;

    while (true)
    {
        if (accept(IsChar('&')))  // captures
        {
            has_captures = true;
            std::string capture;
            if (!name(&capture))
                break;
            else
            {
                space();
                args.push_back(Node(NodeType::Capture, capture));
            }
        }
        else
        {
            std::string symbol;
            if (!name(&symbol))
                break;
            else
            {
                if (has_captures)
                {
                    backtrack(getCount() - symbol.size());
                    error("Captured variables should be at the end of the argument list", symbol);
                }

                space();
                args.push_back(Node(NodeType::Symbol, symbol));
            }
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

    std::optional<Node> func = anyAtomOf({ NodeType::Symbol, NodeType::Field });
    if (!func.has_value())
        return std::nullopt;
    space();

    Node leaf(NodeType::List);
    leaf.push_back(func.value());

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

std::optional<Node> Parser::list()
{
    if (!accept(IsChar('[')))
        return std::nullopt;
    space();

    Node leaf(NodeType::List);
    leaf.push_back(Node(NodeType::Symbol, "list"));

    while (true)
    {
        if (auto value = nodeOrValue(); value.has_value())
        {
            leaf.push_back(value.value());
            space();
        }
        else
            break;
    }

    space();
    expect(IsChar(']'));
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
        // field
        [this]() -> std::optional<Node> {
            std::string symbol;
            if (!name(&symbol))
                return std::nullopt;

            Node leaf = Node(NodeType::Field);
            leaf.push_back(Node(NodeType::Symbol, symbol));

            while (true)
            {
                space();
                if (leaf.list().size() == 1 && !accept(IsChar('.')))  // Symbol:abc
                    return std::nullopt;

                if (leaf.list().size() > 1 && !accept(IsChar('.')))
                    break;
                std::string res;
                if (!name(&res))
                    errorWithNextToken("Expected a field name: <symbol>.<field>");
                leaf.push_back(Node(NodeType::Symbol, res));
            }

            return leaf;
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

std::optional<Node> Parser::anyAtomOf(std::initializer_list<NodeType> types)
{
    auto value = atom();
    if (value.has_value())
    {
        for (auto type : types)
        {
            if (value->nodeType() == type)
                return value;
        }
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
