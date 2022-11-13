#include "combinator.hpp"

#include <iostream>

ParserCombinators::ParserCombinators(const std::string& s) :
    m_in(s), m_count(0), m_row(0), m_col(0)
{
    // if the input string is empty, raise an error
    if (s.size() == 0)
    {
        m_sym = EOF;
        error("Expected symbol, got empty string", "");
    }

    // otherwise, get the first symbol
    next();
}

void ParserCombinators::next()
{
    // getting a character from the stream
    m_sym = m_in[m_count];
    ++m_count;

    int previous_sym = m_count > 0 ? m_in[m_count - 1] : 0;
    if (previous_sym == '\n')
    {
        ++m_row;
        m_col = 0;
    }
    else if (std::isprint(previous_sym))
        ++m_col;
}

void ParserCombinators::backtrack(std::size_t n)
{
    int old_count = m_count;
    m_count = n;
    m_sym = m_in[m_count - 1];

    m_row = 0;
    m_col = 0;
    // adjust the row/col count (this is going to be VERY inefficient)
    for (std::size_t i = 0; i < n; ++i)
    {
        if (m_in[i] == '\n')
        {
            ++m_row;
            m_col = 0;
        }
        else if (std::isprint(m_in[i]))
            ++m_col;
    }
}

bool ParserCombinators::accept(const CharPred& t, std::string* s)
{
    // return false if the predicate couldn't consume the symbol
    if (!t(m_sym))
        return false;
    // otherwise, add it to the string and go to the next symbol
    if (s != nullptr)
        s->push_back(m_sym);
    next();
    return true;
}

bool ParserCombinators::expect(const CharPred& t, std::string* s)
{
    // throw an error if the predicate couldn't consume the symbol
    if (!t(m_sym))
        error("Expected " + t.name, std::string(1, static_cast<char>(m_sym)));  // FIXME downcasting
    // otherwise, add it to the string and go to the next symbol
    if (s != nullptr)
        s->push_back(m_sym);
    next();
    return true;
}

bool ParserCombinators::space(std::string* s)
{
    if (accept(IsSpace))
    {
        if (s != nullptr)
            s->push_back(' ');
        // loop while there are still ' ' to consume
        while (accept(IsSpace));
        return true;
    }
    return false;
}

bool ParserCombinators::inlineSpace(std::string* s)
{
    if (accept(IsInlineSpace))
    {
        if (s != nullptr)
            s->push_back(' ');
        // loop while there are still ' ' to consume
        while (accept(IsInlineSpace));
        return true;
    }
    return false;
}

bool ParserCombinators::endOfLine(std::string* s)
{
    if ((accept(IsChar('\r')) || true) && accept(IsChar('\n')))
    {
        if (s != nullptr)
            s->push_back('\n');
        while ((accept(IsChar('\r')) || true) && accept(IsChar('\n')));
        return true;
    }
    return false;
}

bool ParserCombinators::number(std::string* s)
{
    if (accept(IsDigit, s))
    {
        // consume all the digits available,
        // stop when the symbole isn't a digit anymore
        while (accept(IsDigit, s));
        return true;
    }
    return false;
}

bool ParserCombinators::signedNumber(std::string* s)
{
    return accept(IsMinus, s), number(s);
}

bool ParserCombinators::name(std::string* s)
{
    // first character of a name must be alphabetic
    if (accept(IsAlpha, s))
    {
        // the next ones can be alphanumeric, or '_'
        while (accept(IsAlnum, s) || accept(IsChar('_'), s));
        return true;
    }
    return false;
}

bool ParserCombinators::anyUntil(const CharPred& delim, std::string* s)
{
    if (accept(IsNot(delim), s))
    {
        while (accept(IsNot(delim), s));
        return true;
    }
    return false;
}
