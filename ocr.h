#pragma once

#include <string>
#include <tesseract/baseapi.h>
#include <tesseract/publictypes.h>
#include <leptonica/allheaders.h>
#include <CImg.h>

class OCR {
public:
    OCR(int id);
    ~OCR();
    
    std::string scan();
    
    std::string path;
    
private:
    tesseract::TessBaseAPI *api;
    

};

extern OCR ocr1;
