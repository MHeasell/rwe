#pragma once

#include <memory>
#include <optional>
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

    template <typename T>
    class TdfAdapter
    {
    public:
        using Result = T;
        virtual void onStart() = 0;
        virtual void onProperty(const std::string& name, const std::string& value) = 0;
        virtual void onStartBlock(const std::string& name) = 0;
        virtual void onEndBlock() = 0;
        virtual Result onDone() = 0;
        virtual ~TdfAdapter() = default;
    };

    template <typename It, typename Result>
    class TdfParser
    {
    private:
        LineNormalizingIterator<It> _it;

        std::unique_ptr<TdfAdapter<Result>> adapter;

    public:
        explicit TdfParser(TdfAdapter<Result>* adapter) : adapter(adapter) {}

        Result parse(const It& begin, const It& end)
        {
            _it = LineNormalizingIterator<It>(begin, end);
            return parseInternal();
        }

    private:
        Result parseInternal()
        {
            adapter->onStart();

            consumeWhitespaceAndComments();

            while (!isEndOfFile())
            {
                block();
                consumeWhitespaceAndComments();
            }

            return adapter->onDone();
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
            while (*_it != ']' && *_it != TdfEndOfFile)
            {
                utf8::unchecked::append(*_it, inserter);
                ++_it;
                consumeComments();
            }

            utf8UncheckedTrim(name);

            return name;
        }

        void blockBody()
        {
            expect('{');
            consumeWhitespaceAndComments();
            while (!accept('}'))
            {
                switch (*_it)
                {
                    case '[':
                        block();
                        break;
                    case ';':
                        // Empty statement (i.e. terminator that terminates nothing).
                        // Strictly this isn't really valid
                        // but some files in the wild do have this
                        // and we need cope with them.
                        expect(';');
                        break;
                    default:
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

            if (*_it == '=' || *_it == '\n' || *_it == ';' || *_it == TdfEndOfFile)
            {
                throw TdfParserException(_it.getLine(), _it.getColumn(), "Expected property name");
            }

            utf8::unchecked::append(*_it, inserter);
            ++_it;

            consumeComments();

            while (*_it != '=' && *_it != ';' && *_it != TdfEndOfFile)
            {
                utf8::unchecked::append(*_it, inserter);
                ++_it;
                consumeComments();
            }

            utf8UncheckedTrim(value);

            return value;
        }

        std::string expectPropertyValue()
        {
            std::string value;
            auto inserter = std::back_inserter(value);

            while (*_it != ';' && *_it != TdfEndOfFile)
            {
                utf8::unchecked::append(*_it, inserter);
                ++_it;
                consumeComments();
            }

            utf8UncheckedTrim(value);

            return value;
        }

        bool acceptWhitespace()
        {
            switch (*_it)
            {
                case ' ':
                case '\t':
                case '\n':
                    ++_it;
                    return true;
            }
            return false;
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
            if (!accept2("/*"))
            {
                return false;
            }

            while (!accept2("*/"))
            {
                if (isEndOfFile())
                {
                    throw TdfParserException(_it.getLine(), _it.getColumn(), "Expected */, got end of file");
                }
                ++_it;
            }
            return true;
        }

        bool acceptLineComment()
        {
            if (!accept2("//"))
            {
                return false;
            }
            while (*_it != '\n' && *_it != TdfEndOfFile)
            {
                ++_it;
            }

            return true;
        }

        void expect(TdfCodePoint cp)
        {
            if (!accept(cp))
            {
                throw TdfParserException(_it.getLine(), _it.getColumn(), "Expected " + std::to_string(cp));
            }
        }

        bool accept(TdfCodePoint cp)
        {
            if (*_it != cp)
            {
                return false;
            }

            ++_it;
            return true;
        }

        bool accept2(const char* pTwoChars)
        {
            //operator+ isn't implemented...
            auto localIt = _it;
            if (*_it != pTwoChars[0] || *(++localIt) != pTwoChars[1])
            {
                return false;
            }
            ++_it;
            ++_it;
            return true;
        }

        bool isEndOfFile()
        {
            return _it.isEnd();
        }
    };
}
