#include "bmp.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace bmp;

template<typename ret_t>
ret_t read_bytes(char** s){
    ret_t r = 0;
    for(size_t i = 0; i < sizeof(ret_t); i++){
        uint8_t v = (*s)[i];
        r += v << 8 * i;
    }
    (*s) += sizeof(ret_t);
    return r;
}

template<typename int_t>
void int_to_le(char** byte_packet, int_t myint){
    int_t bit_mask = 0xFF;
    for(size_t i = 0; i < sizeof(int_t); i++){
        (*byte_packet)[i] = (char) ( (myint & bit_mask) >> i * 8);
        bit_mask <<= 8;
    }

    (*byte_packet) += sizeof(int_t);
}


FileHeader::FileHeader(char* s){
    bfType[0] = read_bytes<char>(&s);
    bfType[1] = read_bytes<char>(&s);
    bfSize = read_bytes<uint32_t>(&s);
    bfReserved1 = read_bytes<uint16_t>(&s);
    bfReserved2 = read_bytes<uint16_t>(&s);
    bfOffBits   = read_bytes<uint32_t>(&s);
}


void FileHeader::write(char* s){
        int_to_le<uint8_t>(&s, bfType[0]);
        int_to_le<uint8_t>(&s, bfType[1]);

        int_to_le<uint32_t>(&s, bfSize);
        int_to_le<uint16_t>(&s, bfReserved1);
        int_to_le<uint16_t>(&s, bfReserved2);
        int_to_le<uint32_t>(&s, bfOffBits);
}

ostream& operator<<(ostream& os, const FileHeader& bmfh){
        os << "BITMAP FILE HEADER" << endl;
        os << "Type: " << bmfh.bfType[0] << bmfh.bfType[1] << endl;
        os << "Size: " << bmfh.bfSize << endl;
        os << "Reserved1: " << bmfh.bfReserved1 << " Reserved2: " << bmfh.bfReserved2 << endl;
        os << "Bit offset pix array: " << bmfh.bfOffBits;
        return os;
}


InfoHeader::InfoHeader(char* s){
    biSize          = read_bytes<uint32_t>(&s);
    biWidth         = read_bytes<int>     (&s);
    biHeight        = read_bytes<int>     (&s);
    biPlanes        = read_bytes<uint16_t>(&s);
    biBitCount      = read_bytes<uint16_t>(&s);
    biCompression   = read_bytes<uint32_t>(&s);
    biSizeImage     = read_bytes<uint32_t>(&s);
    biXPelsPerMeter = read_bytes<int>     (&s);
    biYPelsPerMeter = read_bytes<int>     (&s);
    biClrUsed       = read_bytes<uint32_t>(&s);
    biClrImportant  = read_bytes<uint32_t>(&s);
}

void InfoHeader::write(char* s){
    int_to_le<uint32_t>(&s, biSize);
    int_to_le<int>     (&s, biWidth);
    int_to_le<int>     (&s, biHeight);
    int_to_le<uint16_t>(&s, biPlanes);
    int_to_le<uint16_t>(&s, biBitCount);
    int_to_le<uint32_t>(&s, biCompression);
    int_to_le<uint32_t>(&s, biSizeImage);
    int_to_le<int>     (&s, biXPelsPerMeter);
    int_to_le<int>     (&s, biYPelsPerMeter);
    int_to_le<uint32_t>(&s, biClrUsed);
    int_to_le<uint32_t>(&s, biClrImportant);
}

std::ostream& operator<<(std::ostream& os, const InfoHeader& bmih){
    os << "BITMAP INFO HEADER" << endl;
    os << "Size of header (bytes): " << bmih.biSize << endl;
    os << "Resolution: " << bmih.biWidth << "x" << bmih.biHeight << endl;
    os << "Color planes (should be 1):" << bmih.biPlanes << endl;
    os << "Bits per pixel: " << bmih.biBitCount << endl;
    os << "Compression: " << bmih.biCompression << endl;
    os << "Raw size: " << bmih.biSizeImage << endl;
    os << "Resolutin (px/meters): " << bmih.biXPelsPerMeter << "x" << bmih.biYPelsPerMeter << endl;
    os << "Number of colors in palette: " << bmih.biClrUsed << endl;
    os << "Bumber of important colors: " << bmih.biClrImportant << endl;
    return os;
}

Image::Image(int width, int height){
    // initialize headers

    // initialize file header
    file_header.bfType[0] = 'B';
    file_header.bfType[1] = 'M';

    file_header.bfReserved1 = 0;
    file_header.bfReserved2 = 0;

    file_header.bfOffBits = size_headers;

    // initialize the width/height/bbp to calculate the size
    info_header.biBitCount = 24;
    info_header.biWidth = width;
    info_header.biHeight = height;

    // total file size
    size_t rowSize = ceil(info_header.biBitCount * info_header.biWidth / 32.0) * 4;
    file_header.bfSize = abs(height) * rowSize + size_headers;

    // size of info header
    info_header.biSize = size_info_header;
    info_header.biPlanes = 1;
    info_header.biCompression = 0;

    // size of pixel array in bytes
    info_header.biSizeImage = abs(info_header.biWidth) * info_header.biWidth;
    info_header.biXPelsPerMeter = 0;
    info_header.biYPelsPerMeter = 0;
    info_header.biClrUsed = 0;
    info_header.biClrImportant = 0;

    // initialize matrix
    pixelArray = Matrix<Color>(info_header.biWidth, info_header.biHeight);
}


Image::Image(string filename){
    ifstream bmpfile (filename, ios::binary);
    if(bmpfile.is_open()){
        // read file header
        char b_file_header[size_file_header];
        bmpfile.read(b_file_header, size_file_header);

        file_header = FileHeader(b_file_header);

        // read info header
        char dib_header[size_info_header];
        bmpfile.read(dib_header, size_info_header);

        info_header = InfoHeader(dib_header);

        // read the pixel array
        int BytesPerColor = info_header.biBitCount / 8;
        size_t rowSize = ceil(info_header.biBitCount * info_header.biWidth / 32.0) * 4;

        // calculate how many "colors" there are in a row
        size_t clr_x_row = rowSize / BytesPerColor;

        // store the pixelArray in the appropriate matrix
        pixelArray = Matrix<Color>(info_header.biWidth, info_header.biHeight);

        for(size_t nrow = 0; nrow < abs(info_header.biHeight); nrow++){

            // read the row
            char pix_row[rowSize]; char* ptr_pix_row = pix_row;
            bmpfile.read(pix_row, rowSize);

            // read the colors in the rows
            for(size_t ic = 0; ic < clr_x_row; ++ic){

                uint8_t blue = read_bytes<uint8_t>(&ptr_pix_row);
                uint8_t green = read_bytes<uint8_t>(&ptr_pix_row);
                uint8_t red = read_bytes<uint8_t>(&ptr_pix_row);

                Color c = Color(red, green, blue);
                pixelArray.set(ic, info_header.biHeight - 1 - nrow, c);
            }
        }
    }
    else{
        throw ios_base::failure("Opening file something went wrong");
    }
}


void Image::write(string filename){
    char s[file_header.bfSize] = {'n'};
    char* s_ptr = s;

    // write file header
    file_header.write(s_ptr);
    // write info header
    info_header.write(s_ptr + size_file_header);

    // move pixel to the start of the pixelArray position
    s_ptr += size_headers;

    // pad so that is multiple of ints
    // (24 + 24 = 48) -> (48 - 32 + 32 = 64) -> (64 - 48 = 6)
    size_t rowSize = ceil(info_header.biBitCount * info_header.biWidth / 32.0) * 4;
    size_t npad = rowSize - info_header.biWidth*(info_header.biBitCount/8);

    for(size_t nrow = 0; nrow < abs(info_header.biHeight); nrow++){

        // for each pixel write the the byte corresponding to rgb
        for(int ic = 0; ic < info_header.biWidth; ic++){
            Color c = pixelArray.get(ic, info_header.biHeight - 1 - nrow);
            *(s_ptr) = c.b(); s_ptr++;
            *(s_ptr) = c.g(); s_ptr++;
            *(s_ptr) = c.r(); s_ptr++;
        }

        // fill the pad bytes
        for(size_t i = 0; i < npad; i++){
            *(s_ptr) = 0;
            s_ptr++;
        }
    }

    ofstream bmpfile(filename, ios::binary);
    if(bmpfile.is_open()){
        bmpfile.write(s, file_header.bfSize);
    }
    else{
        throw ios_base::failure("Opening file to write went wrong");
    }
}
