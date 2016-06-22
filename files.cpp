#include <string>
#include <iostream>
#include <cstdio>

#include "images.h"

using namespace std;

string single(string filename)
{
    printf("single %s\n", filename.c_str());
    Image img(filename);
    img.search(160);
    //img.search(140);
    printf("finished single\n");
    
    return "";
}
