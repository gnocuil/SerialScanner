#pragma once

#include <string>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <CImg.h>

class OCR {
public:
    OCR(int id);
    ~OCR();
    
    std::string scan();
    
private:
    tesseract::TessBaseAPI *api;
    std::string path;

};

extern OCR ocr;
