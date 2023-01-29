#ifndef PARSER_HPP
#define PARSER_HPP

#include "baseparser.hpp"
#include "node.hpp"
#include "utils.hpp"

#include <string>
#include <optional>
#include <vector>
#include <functional>

class Parser : public BaseParser
{
public:
    Parser(const std::string& code, bool debug);

    void parse();
    const Node& ast() const;

private:
    Node m_ast;
    bool m_debug;

    std::optional<Node> node();
    std::optional<Node> letMutSet();
    std::optional<Node> del();
    std::optional<Node> condition();
    std::optional<Node> loop();
    std::optional<Node> import_();
    std::optional<Node> block();
    std::optional<Node> function();
    std::optional<Node> macro();
    std::optional<Node> functionCall();
    std::optional<Node> list();

    inline std::optional<Node> number()
    {
        auto pos = getCount();

        std::string res;
        if (signedNumber(&res))
        {
            double output;
            if (Utils::isDouble(res, &output))
                return Node(output);
            else
            {
                backtrack(pos);
                error("Is not a valid number", res);
            }
        }
        return std::nullopt;
    }

    inline std::optional<Node> string()
    {
        std::string res;
        if (accept(IsChar('"')))
        {
            while (true)
            {
                if (accept(IsChar('\\')))
                {
                    if (accept(IsChar('"')))
                        res += '\"';
                    else if (accept(IsChar('\\')))
                        res += '\\';
                    else if (accept(IsChar('n')))
                        res += '\n';
                    else if (accept(IsChar('t')))
                        res += '\t';
                    else if (accept(IsChar('v')))
                        res += '\v';
                    else if (accept(IsChar('r')))
                        res += '\r';
                    else if (accept(IsChar('a')))
                        res += '\a';
                    else if (accept(IsChar('b')))
                        res += '\b';
                    else if (accept(IsChar('0')))
                        res += '\0';
                    else
                    {
                        backtrack(getCount() - 1);
                        error("Unknown escape sequence", "\\");
                    }
                }
                else
                    accept(IsNot(IsEither(IsChar('\\'), IsChar('"'))), &res);

                if (accept(IsChar('"')))
                    break;
                else if (isEOF())
                    errorMissingSuffix('"', "string");
                // TODO accept(\Uxxxxx), accept(\uxxxxx)
            }

            return Node(NodeType::String, res);
        }
        return std::nullopt;
    }

    inline std::optional<Node> field()
    {
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
    }

    inline std::optional<Node> symbol()
    {
        std::string res;
        if (!name(&res))
            return std::nullopt;
        return Node(NodeType::Symbol, res);
    }

    std::optional<Node> atom();
    std::optional<Node> anyAtomOf(std::initializer_list<NodeType> types);
    std::optional<Node> nodeOrValue();
    std::optional<Node> wrapped(std::optional<Node> (Parser::*parser)(), const std::string& name, char prefix, char suffix);
};

#endif
