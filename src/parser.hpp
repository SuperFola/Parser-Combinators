#ifndef PARSER_HPP
#define PARSER_HPP

#include "combinator.hpp"
#include "node.hpp"

#include <string>
#include <optional>

class Parser : public ParserCombinators
{
public:
    Parser(const std::string& code);

    void parse();

private:
    Node m_ast;

    bool comment();
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

    std::optional<Node> atom();
    std::optional<Node> anyAtomOf(std::initializer_list<NodeType> types);
    std::optional<Node> nodeOrValue();
    std::optional<Node> wrapped(std::optional<Node> (Parser::*parser)(), char prefix, char suffix);

    void errorWithNextToken(const std::string& message);
};

#endif
