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
#include <iostream>
#include <ostream>

#include <png.hpp>

template< class pixel >
void
generate_image(png::image< pixel >& image, char const* filename)
{
    for (size_t j = 0; j < image.get_height(); ++j)
    {
        for (size_t i = 0; i < image.get_width(); ++i)
        {
            image.set_pixel(i, j, pixel(i + j));
        }
    }
    image.write(filename);
}

int
main(int argc, char* argv[])
try
{
    size_t const width = 32;
    size_t const height = 32;

    png::image< png::gray_pixel_1 > image1(width, height);
    generate_image(image1, "out/gray_packed_1.png.out");

    png::image< png::gray_pixel_2 > image2(width, height);
    generate_image(image2, "out/gray_packed_2.png.out");

    png::image< png::gray_pixel_4 > image4(width, height);
    generate_image(image4, "out/gray_packed_4.png.out");
}
catch (std::exception const& error)
{
    std::cerr << "generate_gray_packed: " << error.what() << std::endl;
    return EXIT_FAILURE;
}
