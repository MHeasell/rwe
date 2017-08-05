#include <iostream>
#include <ostream>

#include <png.hpp>

int
main()
{
    std::cerr << "read_write_param: do not run this test--it's compile-only"
              << std::endl;
    exit(1);

    typedef png::image< png::rgb_pixel > image;
    image image1;               // default contructor
    image image2("test2.png");  // char const*
    char test3[] = "test3.png"; // char*
    image image3(test3); 
    image image4(std::string("test4.png"));

    image1.write("test4.png");  // char const*
    char test5[] = "test5.png";
    image1.write(test5);        // char*
    image1.write(std::string("test6.png"));
}
