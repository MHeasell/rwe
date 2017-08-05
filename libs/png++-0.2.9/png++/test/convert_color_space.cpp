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

void
print_usage()
{
    std::cerr << "usage: convert_color_space RGB|RGBA|GRAY|GA 8|16 PB|PB2"
              << " INFILE OUTFILE" << std::endl;
}

template< typename pixel >
void
convert_image(char const* buffer_type, char const *infile, char const* outfile)
{
    if (strcmp(buffer_type, "PB")) {
        png::image< pixel, png::pixel_buffer< pixel > > image(infile);
        image.write(outfile);
    } else {
        png::image< pixel, png::solid_pixel_buffer< pixel > > image(infile);
        image.write(outfile);
    }
}

int
main(int argc, char* argv[])
try
{
    if (argc != 6)
    {
        print_usage();
        return EXIT_FAILURE;
    }
    
    char const* space = argv[1];
    int bits = atoi(argv[2]);
    char const* buffer_type = argv[3];
    char const* infile = argv[4];
    char const* outfile = argv[5];

    if (bits != 8 && bits != 16)
    {
        print_usage();
        return EXIT_FAILURE;
    }
    if (strcmp(space, "RGB") == 0)
    {
        if (bits == 16)
        {
            convert_image< png::rgb_pixel_16 >(buffer_type, infile, outfile);
        }
        else
        {
            convert_image< png::rgb_pixel >(buffer_type, infile, outfile);
        }
    }
    else if (strcmp(space, "RGBA") == 0)
    {
        if (bits == 16)
        {
            convert_image< png::rgba_pixel_16 >(buffer_type, infile, outfile);
        }
        else
        {
            convert_image< png::rgba_pixel >(buffer_type, infile, outfile);
        }
    }
    else if (strcmp(space, "GRAY") == 0)
    {
        if (bits == 16)
        {
            convert_image< png::gray_pixel_16 >(buffer_type, infile, outfile);
        }
        else
        {
            convert_image< png::gray_pixel >(buffer_type, infile, outfile);
        }
    }
    else if (strcmp(space, "GA") == 0)
    {
        if (bits == 16)
        {
            png::ga_pixel_16 ga(1); // test alpha_pixel_traits
            
            convert_image< png::ga_pixel_16 >(buffer_type, infile, outfile);
        }
        else
        {
            png::ga_pixel ga(1); // test alpha_pixel_traits
            
            convert_image< png::ga_pixel >(buffer_type, infile, outfile);
        }
    }
    else
    {
        print_usage();
        return EXIT_FAILURE;
    }
}
catch (std::exception const& error)
{
    std::cerr << "convert_color_space: " << error.what() << std::endl;
    return EXIT_FAILURE;
}
