#include <iostream>
#include <cstdio>
#include <string>
#include <CImg.h>
#include "ocr.h"

using namespace std;

OCR ocr(1);

OCR::OCR(int id)
{

    char buf[100] = {0};
    sprintf(buf, "debug/ocr_%d.jpg", id);
    path = buf;

    api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

}

string OCR::scan()
{
    char *outText;
    // Open input image with leptonica library
    Pix *image = pixRead(path.c_str());
    api->SetPageSegMode(PSM_SINGLE_CHAR);
    api->SetImage(image);
    // Get OCR result
    outText = api->GetUTF8Text();
    printf("OCR output:\n%s", outText);
    string ret = outText;

    delete [] outText;
    pixDestroy(&image);
    return ret;
}

OCR::~OCR()
{
    // Destroy used object and release memory
    api->End();

}
