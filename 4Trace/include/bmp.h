#ifndef BMP_H
#define BMP_H

#include <cstdint>
#include <iostream>

#include "mat.tpp"
#include "vec.tpp"


namespace bmp{

    struct FileHeader{

        char bfType[2];
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;

        FileHeader() {};
        FileHeader(char* s);
        void write(char* s);
    };


    struct InfoHeader{
        uint32_t    biSize;
        int         biWidth;
        int         biHeight;
        uint16_t    biPlanes;
        uint16_t    biBitCount;
        uint32_t    biCompression;
        uint32_t    biSizeImage;
        int         biXPelsPerMeter;
        int         biYPelsPerMeter;
        uint32_t    biClrUsed;
        uint32_t    biClrImportant;

        InfoHeader() {};
        InfoHeader(char* s);
        void write(char* s);
    };


    class Color : V3<uint8_t>{
    public:

        Color(uint8_t c) : V3(c) {}
        Color(uint8_t r, uint8_t g, uint8_t b) : V3(r, g, b) {}

        Color(V3d c) : V3( (uint8_t) c.x(), (uint8_t) c.y(), (uint8_t) c.z() ) {};

        uint8_t r(){return x();}
        uint8_t g(){return y();}
        uint8_t b(){return z();}
    };


    class Image{
    private:
        constexpr static size_t size_file_header = 14;
        constexpr static size_t size_info_header = 40;
        constexpr static size_t size_headers = size_file_header + size_info_header;

    public:
        // public members
        FileHeader file_header;
        InfoHeader info_header;
        Matrix<Color> pixelArray;

        // c'tors
        Image(int width, int height);
        Image(std::string filename);

        // save image to file
        void write(std::string filename);
    };

}


std::ostream& operator<<(std::ostream& os, const bmp::FileHeader& bmfh);
std::ostream& operator<<(std::ostream& os, const bmp::InfoHeader& bmih);


#endif // BMP_H
