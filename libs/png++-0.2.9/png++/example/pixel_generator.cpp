/*
 * Copyright (C) 2007,2008   Alex Shulgin
 *
 * This file is part of png++ the C++ wrapper for libpng.  PNG++ is free
 * software; the exact copying conditions are as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <cstdlib>
#include <iostream>
#include <ostream>

#include <png.hpp>

class pixel_generator
    : public png::generator< png::gray_pixel_1, pixel_generator >
{
public:
    pixel_generator(size_t width, size_t height)
        : png::generator< png::gray_pixel_1, pixel_generator >(width, height),
          m_row(width)
    {
        for (size_t i = 0; i < m_row.size(); ++i)
        {
            m_row[i] = i > m_row.size() / 2 ? 1 : 0;
        }
    }

    png::byte* get_next_row(size_t /*pos*/)
    {
        size_t i = std::rand() % m_row.size();
        size_t j = std::rand() % m_row.size();
        png::gray_pixel_1 t = m_row[i];
        m_row[i] = m_row[j];
        m_row[j] = t;
        return reinterpret_cast< png::byte* >(row_traits::get_data(m_row));
    }

private:
    typedef png::packed_pixel_row< png::gray_pixel_1 > row;
	typedef png::row_traits< row > row_traits;
	row m_row;
};

int
main()
try
{
    size_t const width = 32;
    size_t const height = 512;

    std::ofstream file("generated.png", std::ios::binary);
    pixel_generator generator(width, height);
    generator.write(file);
}
catch (std::exception const& error)
{
    std::cerr << "pixel_generator: " << error.what() << std::endl;
    return EXIT_FAILURE;
}
