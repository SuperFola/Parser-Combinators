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

    inline void backtrack(std::size_t count) { back(getCount() - count + 1); }
};

#endif
