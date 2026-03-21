#pragma once

#include <istream>

namespace rwe
{
    class SpanStreamBuf : public std::streambuf
    {
    public:
        SpanStreamBuf() = default;

        SpanStreamBuf(const char* data, std::size_t size)
        {
            reset(data, size);
        }

        void reset(const char* data, std::size_t size)
        {
            auto p = const_cast<char*>(data);
            setg(p, p, p + size);
        }

    protected:
        std::streampos seekoff(std::streamoff off, std::ios_base::seekdir dir, std::ios_base::openmode which) override
        {
            if (!(which & std::ios_base::in))
            {
                return std::streampos(-1);
            }

            char* newPos;
            switch (dir)
            {
                case std::ios_base::beg:
                    newPos = eback() + off;
                    break;
                case std::ios_base::cur:
                    newPos = gptr() + off;
                    break;
                case std::ios_base::end:
                    newPos = egptr() + off;
                    break;
                default:
                    return std::streampos(-1);
            }

            if (newPos < eback() || newPos > egptr())
            {
                return std::streampos(-1);
            }

            setg(eback(), newPos, egptr());
            return std::streampos(newPos - eback());
        }

        std::streampos seekpos(std::streampos pos, std::ios_base::openmode which) override
        {
            return seekoff(std::streamoff(pos), std::ios_base::beg, which);
        }
    };

    class SpanStream : public std::istream
    {
    private:
        SpanStreamBuf buf;

    public:
        SpanStream(const char* data, std::size_t size)
            : std::istream(nullptr), buf(data, size)
        {
            rdbuf(&buf);
        }
    };
}
