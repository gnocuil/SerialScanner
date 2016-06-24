#pragma once

#include <string>
#include <vector>
#include <CImg.h>

using namespace cimg_library;

const int NUM1 = 8;
const int NUM2 = 8;

class Image {
public:
    //Image();
    Image(std::string filename);//image obj from a local image file
    Image(const CImg<unsigned char>& img_, Image* parent, std::string prefix_);//sub-image obj from another Image
    ~Image();
    
    std::vector<std::string> search(int threshold_);
    
    std::string recognize();
    
    std::string recognize_2();
    
    //int countLine();
    
    std::vector<Image> cut();
    
    Image subImage(int h1, int h2, int id);
    
    void rotate();
    
    void save(std::string suffix);
    
    int count(const CImg<unsigned char>& image);
    int countLine(const CImg<unsigned char>& image, int line);
    
    void findChar();
    
    char ocr(const CImg<unsigned char>& img);
    std::string ocr16(const CImg<unsigned char>& img);
    
    std::string prefix;
    
    int threshold;
    
    int debug;
    
private:
    CImg<unsigned char> image;
    //int w, h;
    
    //int s_h, s_w;
    //int **s;
};

struct Point {
    int w,h;
    Point(int _w, int _h): w(_w),h(_h) {
    }
};
