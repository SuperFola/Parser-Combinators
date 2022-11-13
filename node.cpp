#include "node.hpp"

Node::Node(NodeType type) :
    m_type(type)
{}

Node::Node(NodeType type, const std::string& s) :
    m_value(s), m_type(type)
{}

Node::Node(double d) :
    m_value(d), m_type(NodeType::Number)
{}

Node::Node(long l) :
    m_value(static_cast<double>(l)), m_type(NodeType::Number)
{}

Node::Node(int i) :
    m_value(static_cast<double>(i)), m_type(NodeType::Number)
{}

Node::Node(const std::vector<Node>& n) :
    m_value(n), m_type(NodeType::List)
{}

void Node::push_back(const Node& n)
{
    std::get<std::vector<Node>>(m_value).push_back(n);
}
