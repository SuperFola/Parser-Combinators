#ifndef NODE_HPP
#define NODE_HPP

#include <variant>
#include <string>
#include <vector>
#include <ostream>

enum class NodeType
{
    Symbol,
    Capture,
    GetField,
    Keyword,
    String,
    Number,
    List,
    Spread,
    Unused
};

class Node
{
public:
    using Value = std::variant<double, std::string, std::vector<Node>>;

    Node(NodeType type);
    Node(NodeType type, const std::string& str);
    Node(double d);
    Node(long l);
    Node(int i);
    Node(const std::vector<Node>& n);

    inline NodeType nodeType() const { return m_type; }

    double number() const { return std::get<double>(m_value); }
    const std::string& string() const { return std::get<std::string>(m_value); }
    const std::vector<Node>& list() const { return std::get<std::vector<Node>>(m_value); }

    void push_back(const Node& n);

    friend std::ostream& operator<<(std::ostream& os, const Node& node);

private:
    Value m_value;
    NodeType m_type;
};

#endif
