#include "node.hpp"

Node::Node(NodeType type) :
    m_type(type)
{
    switch (m_type)
    {
        case NodeType::List:
            m_value = std::vector<Node>();
            break;

        case NodeType::Symbol:
        case NodeType::Capture:
        case NodeType::GetField:
        case NodeType::Keyword:
        case NodeType::String:
        case NodeType::Spread:
            m_value = "";
            break;

        case NodeType::Number:
            m_value = 0.0;
            break;

        default:
            break;
    }
}

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

std::ostream& operator<<(std::ostream& os, const Node& node)
{
    switch (node.nodeType())
    {
        case NodeType::Symbol:
            os << "Symbol:" << node.string();
            break;

        case NodeType::Capture:
            os << "Capture:" << node.string();
            break;

        case NodeType::GetField:
            os << "GetField:" << node.string();
            break;

        case NodeType::Keyword:
            os << "Keyword:" << node.string();
            break;

        case NodeType::String:
            os << "String:" << node.string();
            break;

        case NodeType::Number:
            os << "Number:" << node.number();
            break;

        case NodeType::List:
            os << "( ";
            for (std::size_t i = 0, end = node.list().size(); i < end; ++i)
                os << node.list()[i] << " ";
            os << ")";
            break;

        case NodeType::Spread:
            os << "Spread:" << node.string();
            break;

        case NodeType::Unused:
            os << "Unused:" << node.string();
            break;
    }
    return os;
}
