#ifndef SRC_COMBINATOR_HPP
#define SRC_COMBINATOR_HPP

#include <string>
#include <exception>
#include <stdexcept>
#include <utility>
#include <cctype>
#include <initializer_list>

// Base for all the Character Predicates used by the parsers
// Those act as 'rules' for the parsers
struct CharPred
{
    // storing a name to identify the predicates in the parsers
    const std::string name;

    CharPred(const std::string& n) :
        name(n) {}

    virtual bool operator()(const char c) const = 0;
};

inline struct IsSpace : public CharPred
{
    IsSpace() :
        CharPred("space") {}
    virtual bool operator()(const char c) const override
    {
        return std::isspace(c) != 0;
    }
} IsSpace;

inline struct IsInlineSpace : public CharPred
{
    IsInlineSpace() :
        CharPred("inline space") {}
    virtual bool operator()(const char c) const override
    {
        return (std::isspace(c) != 0) && (c != '\n') && (c != '\r');
    }
} IsInlineSpace;

inline struct IsDigit : public CharPred
{
    IsDigit() :
        CharPred("digit") {}
    virtual bool operator()(const char c) const override
    {
        return std::isdigit(c) != 0;
    }
} IsDigit;

inline struct IsUpper : public CharPred
{
    IsUpper() :
        CharPred("uppercase") {}
    virtual bool operator()(const char c) const override
    {
        return std::isupper(c) != 0;
    }
} IsUpper;

inline struct IsLower : public CharPred
{
    IsLower() :
        CharPred("lowercase") {}
    virtual bool operator()(const char c) const override
    {
        return std::islower(c) != 0;
    }
} IsLower;

inline struct IsAlpha : public CharPred
{
    IsAlpha() :
        CharPred("alphabetic") {}
    virtual bool operator()(const char c) const override
    {
        return std::isalpha(c) != 0;
    }
} IsAlpha;

inline struct IsAlnum : public CharPred
{
    IsAlnum() :
        CharPred("alphanumeric") {}
    virtual bool operator()(const char c) const override
    {
        return std::isalnum(c) != 0;
    }
} IsAlnum;

inline struct IsPrint : public CharPred
{
    IsPrint() :
        CharPred("printable") {}
    virtual bool operator()(const char c) const override
    {
        return std::isprint(c) != 0;
    }
} IsPrint;

struct IsChar : public CharPred
{
    explicit IsChar(const char c) :
        CharPred("'" + std::string(1, c) + "'"), m_k(c)
    {}
    virtual bool operator()(const char c) const override
    {
        return m_k == c;
    }

private:
    const char m_k;
};

struct IsEither : public CharPred
{
    explicit IsEither(const CharPred& a, const CharPred& b) :
        CharPred("(" + a.name + " | " + b.name + ")"), m_a(a), m_b(b)
    {}
    virtual bool operator()(const char c) const override
    {
        return m_a(c) || m_b(c);
    }

private:
    const CharPred& m_a;
    const CharPred& m_b;
};

struct IsNot : public CharPred
{
    explicit IsNot(const CharPred& a) :
        CharPred("~" + a.name), m_a(a)
    {}
    virtual bool operator()(const char c) const override
    {
        return !m_a(c);
    }

private:
    const CharPred& m_a;
};

inline struct IsAny : public CharPred
{
    IsAny() :
        CharPred("any") {}
    virtual bool operator()(const char) const override
    {
        return true;
    }
} IsAny;

const IsChar IsMinus('-');

struct ParseError : public std::runtime_error
{
    const std::size_t line;
    const std::size_t col;
    const std::string expr;
    const char symbol;

    ParseError(const std::string& what, std::size_t lineNum, std::size_t column, std::string exp, char sym) :
        std::runtime_error(what), line(lineNum), col(column), expr(std::move(exp)), symbol(sym)
    {}
};

class ParserCombinators
{
public:
    ParserCombinators(const std::string& s);

private:
    std::string m_in;
    std::size_t m_count;
    std::size_t m_row;
    std::size_t m_col;
    char m_sym;

    /*
        getting next character and changing the values of count/row/col/sym
    */
    void next();

protected:
    inline void error(const std::string& error, const std::string exp)
    {
        throw ParseError(error, m_row, m_col, exp, m_sym);
    }

    // basic getters
    inline std::size_t getCol() { return m_col; }
    inline std::size_t getRow() { return m_row; }
    inline std::size_t getCount() { return m_count; }
    inline std::size_t getSize() { return m_in.size(); }
    inline bool isEOF() { return m_count >= m_in.size() || m_sym == '\0'; }

    void backtrack(std::size_t n);

    /*
        Function to use and check if a Character Predicate was able to parse
        the current symbol.
        Add the symbol to the given string (if there was one) and call next()
    */
    bool accept(const CharPred& t, std::string* s = nullptr);

    /*
        Function to use and check if a Character Predicate was able to parse
        the current Symbol.
        Add the symbol to the given string (if there was one) and call next().
        Throw a ParseError if it couldn't.
    */
    bool expect(const CharPred& t, std::string* s = nullptr);

    // basic parsers
    bool space(std::string* s = nullptr);
    bool inlineSpace(std::string* s = nullptr);
    bool endOfLine(std::string* s = nullptr);
    bool number(std::string* s = nullptr);
    bool signedNumber(std::string* s = nullptr);
    bool name(std::string* s = nullptr);
    bool anyUntil(const CharPred& delim, std::string* s = nullptr);

    bool oneOf(std::initializer_list<std::string> words, std::string* s = nullptr);
};

#endif
