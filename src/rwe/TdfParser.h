#ifndef RWE_TDFPARSER_H
#define RWE_TDFPARSER_H

#include <boost/optional.hpp>
#include <rwe/rwe_string.h>
#include <stdexcept>
#include <string>
#include <utf8.h>

namespace rwe
{
    using TdfCodePoint = unsigned int;

    const TdfCodePoint TdfEndOfFile = U'\u0004';

    template <typename Iterator>
    class LineNormalizingIterator
    {
    private:
        Iterator it;
        Iterator end;
        std::size_t line;
        std::size_t column;

    public:
        LineNormalizingIterator() : line(1), column(1) {}
        LineNormalizingIterator(const Iterator& it, const Iterator& end) : it(it), end(end), line(1), column(1) {}

        std::size_t getLine() { return line; }
        std::size_t getColumn() { return column; }

        bool isEnd() const { return it == end; }

        LineNormalizingIterator& operator++()
        {
            if (isEnd())
            {
                return *this;
            }

            auto val = *it;
            ++it;

            if (val == '\r')
            {
                ++line;
                column = 1;

                // skip forward over \r\n as if it were one character
                if (!isEnd() && *it == '\n')
                {
                    ++it;
                }
            }
            else if (val == '\n')
            {
                ++line;
                column = 1;
            }
            else
            {
                ++column;
            }

            return *this;
        }

        TdfCodePoint operator*() const
        {
            if (isEnd())
            {
                return TdfEndOfFile;
            }

            auto val = *it;

            // pretend \r is \n
            if (val == '\r')
            {
                return '\n';
            }

            return val;
        }
    };

    class TdfParserException : public std::runtime_error
    {
    private:
        std::size_t line;
        std::size_t column;
    public:
        explicit TdfParserException(std::size_t line, std::size_t column, const char* message);
        explicit TdfParserException(std::size_t line, std::size_t column, const std::string& message);
    };

    class TdfAdapter
    {
    public:
        virtual void onStart() = 0;
        virtual void onProperty(const std::string& name, const std::string& value) = 0;
        virtual void onStartBlock(const std::string& name) = 0;
        virtual void onEndBlock() = 0;
        virtual void onDone() = 0;
    };

    template <typename It>
    class TdfParser
    {
    private:
        LineNormalizingIterator<It> _it;

        TdfAdapter* adapter;

    public:
        void parse(const It& begin, const It& end, TdfAdapter& adapter)
        {
            _it = LineNormalizingIterator<It>(begin, end);
            this->adapter = &adapter;

            parseInternal();
        }

    private:
        void parseInternal()
        {
            adapter->onStart();

            consumeWhitespaceAndComments();

            while (!isEndOfFile())
            {
                block();
                consumeWhitespaceAndComments();
            }

            adapter->onDone();
        }
        void block()
        {
            auto title = blockHead();

            adapter->onStartBlock(title);

            consumeWhitespaceAndComments();
            blockBody();

            adapter->onEndBlock();
        }

        std::string blockHead()
        {
            expect('[');
            consumeWhitespaceAndComments();
            auto name = blockName();
            consumeWhitespaceAndComments();
            expect(']');

            return name;
        }

        std::string blockName()
        {
            std::string name;
            auto inserter = std::back_inserter(name);

            consumeComments();
            while (auto cp = acceptNotAny(std::vector<TdfCodePoint>{']', TdfEndOfFile}))
            {
                utf8::append(*cp, inserter);
                consumeComments();
            }

            utf8Trim(name);

            return name;
        }

        void blockBody()
        {
            expect('{');
            consumeWhitespaceAndComments();
            while (!accept('}'))
            {
                if (peek() == '[')
                {
                    block();
                }
                else
                {
                    property();
                }

                consumeWhitespaceAndComments();
            }
        }

        void property()
        {
            auto name = expectPropertyName();
            consumeWhitespaceAndComments();
            expect('=');
            consumeWhitespaceAndComments();
            auto value = expectPropertyValue();
            consumeWhitespaceAndComments();
            expect(';');

            adapter->onProperty(name, value);
        }

        std::string expectPropertyName()
        {
            std::string value;
            auto inserter = std::back_inserter(value);

            auto firstCodePoint = acceptNotAny(std::vector<TdfCodePoint>{'=', '\n', ';', TdfEndOfFile});
            if (!firstCodePoint)
            {
                throw TdfParserException(_it.getLine(), _it.getColumn(), "Expected property name");
            }
            utf8::append(*firstCodePoint, inserter);

            consumeComments();
            while (auto cp = acceptNotAny(std::vector<TdfCodePoint>{'=', '\n', ';', TdfEndOfFile}))
            {
                utf8::append(*cp, inserter);
                consumeComments();
            }

            utf8Trim(value);

            return value;
        }

        std::string expectPropertyValue()
        {
            std::string value;
            auto inserter = std::back_inserter(value);

            auto firstCodePoint = acceptNotAny(std::vector<TdfCodePoint>{';', TdfEndOfFile});
            if (!firstCodePoint)
            {
                throw TdfParserException(_it.getLine(), _it.getColumn(), "Expected property value");
            }
            utf8::append(*firstCodePoint, inserter);

            consumeComments();
            while (auto cp = acceptNotAny(std::vector<TdfCodePoint>{';', TdfEndOfFile}))
            {
                utf8::append(*cp, inserter);
                consumeComments();
            }

            utf8Trim(value);

            return value;
        }

        bool acceptWhitespace()
        {
            return accept(' ') || accept('\t') || accept('\n');
        }

        void consumeWhitespaceAndComments()
        {
            while (acceptWhitespace() || acceptComment())
            {
                // do nothing
            }
        }

        void consumeComments()
        {
            while (acceptComment())
            {
                // do nothing
            }
        }

        bool acceptComment()
        {
            return acceptLineComment() || acceptBlockComment();
        }

        bool acceptBlockComment()
        {
            if (!accept("/*"))
            {
                return false;
            }

            while (true)
            {
                if (accept("*/"))
                {
                    return true;
                }

                if (acceptNotEndOfFile())
                {
                    continue;
                }

                throw TdfParserException(_it.getLine(), _it.getColumn(), "Expected */, got end of file");
            }
        }

        bool acceptLineComment()
        {
            if (!accept("//"))
            {
                return false;
            }

            while (acceptNotAny(std::vector<TdfCodePoint> {'\n', TdfEndOfFile}))
            {
                // do nothing
            }

            return true;
        }

        bool acceptNotEndOfFile()
        {
            if (isEndOfFile())
            {
                return false;
            }

            next();
            return true;
        }

        void expect(TdfCodePoint cp)
        {
            if (!accept(cp))
            {
                throw TdfParserException(_it.getLine(), _it.getColumn(), "Expected " + cp);
            }
        }

        template <typename Container>
        boost::optional<TdfCodePoint> acceptNotAny(const Container& container)
        {
            return acceptNotAny(container.begin(), container.end());
        }

        template <typename CharIt>
        boost::optional<TdfCodePoint> acceptNotAny(const CharIt& begin, const CharIt& end)
        {
            auto curr = peek();
            auto it = std::find(begin, end, curr);
            if (it == end)
            {
                next();
                return curr;
            }

            return boost::none;
        }

        boost::optional<TdfCodePoint> acceptNot(TdfCodePoint cp)
        {
            auto val = peek();

            if (val == cp)
            {
                return boost::none;
            }

            next();
            return val;
        }

        bool accept(TdfCodePoint cp)
        {
            if (peek() != cp)
            {
                return false;
            }

            next();
            return true;
        }

        bool accept(const std::string& str)
        {
            auto localIt = _it;
            for (auto it = utf8Begin(str), end = utf8End(str); it != end; ++it)
            {
                if (*localIt != *it)
                {
                    return false;
                }

                ++localIt;
            }

            _it = localIt;
            return true;
        }

        bool isEndOfFile()
        {
            return _it.isEnd();
        }

        TdfCodePoint peek()
        {
            return *_it;
        }

        TdfCodePoint next()
        {
            auto val = *_it;
            ++_it;
            return val;
        }
    };
}

#endif
