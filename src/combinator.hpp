#ifndef SRC_COMBINATOR_HPP
#define SRC_COMBINATOR_HPP

#include <string>
#include <exception>
#include <stdexcept>
#include <utility>
#include <cctype>

// Base for all the Character Predicates used by the parsers
// Those act as 'rules' for the parsers
struct CharPred
{
    // storing a name to identify the predicates in the parsers
    const std::string name;

    CharPred(const std::string& n) :
        name(n) {}
    // the int c represents a character
    virtual bool operator()(const int c) const = 0;
};

inline struct IsSpace : public CharPred
{
    IsSpace() :
        CharPred("space") {}
    virtual bool operator()(const int c) const override
    {
        return std::isspace(c) != 0;
    }
} IsSpace;

inline struct IsInlineSpace : public CharPred
{
    IsInlineSpace() :
        CharPred("inline space") {}
    virtual bool operator()(const int c) const override
    {
        return (std::isspace(c) != 0) && (c != '\n') && (c != '\r');
    }
} IsInlineSpace;

inline struct IsDigit : public CharPred
{
    IsDigit() :
        CharPred("digit") {}
    virtual bool operator()(const int c) const override
    {
        return std::isdigit(c) != 0;
    }
} IsDigit;

inline struct IsUpper : public CharPred
{
    IsUpper() :
        CharPred("uppercase") {}
    virtual bool operator()(const int c) const override
    {
        return std::isupper(c) != 0;
    }
} IsUpper;

inline struct IsLower : public CharPred
{
    IsLower() :
        CharPred("lowercase") {}
    virtual bool operator()(const int c) const override
    {
        return std::islower(c) != 0;
    }
} IsLower;

inline struct IsAlpha : public CharPred
{
    IsAlpha() :
        CharPred("alphabetic") {}
    virtual bool operator()(const int c) const override
    {
        return std::isalpha(c) != 0;
    }
} IsAlpha;

inline struct IsAlnum : public CharPred
{
    IsAlnum() :
        CharPred("alphanumeric") {}
    virtual bool operator()(const int c) const override
    {
        return std::isalnum(c) != 0;
    }
} IsAlnum;

inline struct IsPrint : public CharPred
{
    IsPrint() :
        CharPred("printable") {}
    virtual bool operator()(const int c) const override
    {
        return std::isprint(c) != 0;
    }
} IsPrint;

struct IsChar : public CharPred
{
    explicit IsChar(const char c) :
        m_k(c), CharPred("'" + std::string(1, c) + "'")
    {}
    virtual bool operator()(const int c) const override
    {
        return m_k == c;
    }

private:
    const int m_k;
};

struct IsEither : public CharPred
{
    explicit IsEither(const CharPred& a, const CharPred& b) :
        m_a(a), m_b(b), CharPred("(" + a.name + " | " + b.name + ")")
    {}
    virtual bool operator()(const int c) const override
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
        m_a(a), CharPred("~" + a.name)
    {}
    virtual bool operator()(const int c) const override
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
    virtual bool operator()(const int c) const override
    {
        return true;
    }
} IsAny;

const IsChar IsMinus('-');

struct ParseError : public std::runtime_error
{
    const int row;
    const int col;
    const std::string exp;
    const int sym;

    ParseError(const std::string& what, int row, int col, std::string exp, int sym) :
        std::runtime_error(what), row(row), col(col), exp(std::move(exp)), sym(sym)
    {}
};

class ParserCombinators
{
public:
    ParserCombinators(const std::string& s);

private:
    std::string m_in;
    int m_count;
    int m_row;
    int m_col;
    int m_sym;

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
    inline int getCol() { return m_col; }
    inline int getRow() { return m_row; }
    inline int getCount() { return m_count; }
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
};

#endif
