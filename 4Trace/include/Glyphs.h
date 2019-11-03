#ifndef GLYPHS_H
#define GLYPHS_H

#include <vector>
#include <string>

#include "vec.tpp"
#include "bmp.h"

class Glyphs{
private:

    bmp::Image base;
    const size_t dimx = 32;
    const size_t dimy = 32;
    const size_t rows = 8;
    const size_t cols = 16;

    std::vector<bmp::Image> images;

    bmp::Image cut_glyph(size_t icol, size_t irow);

public:
    Glyphs();

    bmp::Image get_char(char c);

    void imprint(bmp::Image& imp_image, char c, V2<size_t> position, double scale);

    void imprint(bmp::Image& imp_image, std::string str, V2<size_t> position, double scale);


};

#endif // GLYPHS_H
