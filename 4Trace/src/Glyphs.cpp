#include "Glyphs.h"

#include <cmath>

using namespace std;


Glyphs::Glyphs(){
    base = bmp::Image("./bmp_font/bmp_if_font_5.bmp");

    for(size_t irow = 0; irow < rows; irow++){
        for(size_t icol = 0; icol < cols; icol++){
            bmp::Image glyph;
            glyph = cut_glyph(icol, irow);

            images.push_back(glyph);
        }
    }
}

bmp::Image Glyphs::cut_glyph(size_t icol, size_t irow){

    bmp::Image glyph(dimx, dimy);

    for(size_t i = 0; i < dimx; i++){
        for(size_t j = 0; j < dimy; j++){

            bmp::Color px = base.pixelArray.get(i + dimx * icol, j + dimy * irow);

            glyph.pixelArray.set(i, j, px);
        }
    }

    return glyph;
}

bmp::Image Glyphs::get_char(char c){
    size_t pos = (int) c;
    if(pos < 96){pos -= 1;}
    return images[pos];
}

void Glyphs::imprint(bmp::Image& imp_image, char c, V2<size_t> position, double scale){

    bmp::Image character = get_char(c);
    bmp::Image scaled;

    size_t dimx_char = int(double(dimx) * scale);
    size_t dimy_char = int(double(dimy) * scale);

    if(scale != 1){


        // scale
        scaled = bmp::Image(dimx_char, dimy_char);

        for(size_t i = 0; i < dimx_char; i++){
            for(size_t j = 0; j < dimy_char; j++){
                size_t src_x = double(i) / double(dimx_char) * dimx;
                size_t src_y = double(j) / double(dimy_char) * dimy;

                src_x = min( (size_t) dimx - 1, src_x);
                src_y = min( (size_t) dimy - 1, src_y);

                bmp::Color px = character.pixelArray.get(src_x, src_y);
                scaled.pixelArray.set(i, j, px);
            }
        }
    }
    else{
        scaled = character;
    }



    for(size_t i = 0; i < dimx_char; i++){
        for(size_t j = 0; j < dimy_char; j++){
            bmp::Color px = scaled.pixelArray.get(i, j);
            size_t xpos = i + position.x();
            size_t ypos = j + position.y();

            bool check_border_x = (xpos < (size_t) imp_image.width() );
            bool check_border_y = (ypos < (size_t) imp_image.height());

            if(check_border_x && check_border_y ){
                imp_image.pixelArray.set(xpos, ypos, px);
            }
        }
    }
}

void Glyphs::imprint(bmp::Image& imp_image, string str, V2<size_t> position, double scale){

    size_t dimx_char = int(double(dimx) * scale);

    //size_t dimy_char = int(double(dimy) * scale);

    for(size_t i = 0; i < str.size(); i++){
        V2<size_t> char_pos = position;
        char_pos.x() = position.x() + i * dimx_char;
        imprint(imp_image, str[i], char_pos, scale);
    }
}
