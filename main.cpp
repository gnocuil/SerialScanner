#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "files.h"

using namespace std;

void help()
{
    puts("Usage: serialscanner [options]");
    puts("options:       -i image_filename       <specify one input image (jpg/png) filename>");
    puts("               -f folder_name          <specify one folder (scan all jpg/png files inside)>");
    exit(0);
}

int main(int argc, char **argv)
{
    string filename = "";
    string folder = "";
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i],"-i")==0 && i+1<argc) {
            filename=argv[i+1];
            printf("input image filename: %s\n", filename.c_str());
            ++i;
        } else if (strcmp(argv[i],"-f")==0 && i+1<argc) {
            ++i;
            folder=argv[i];
            if (folder.size()>1 && folder[folder.size()-1]=='/')
                folder.resize(folder.size()-1);
            printf("folder: %s\n", folder.c_str());
        } else {
            help();
        }
    }
    
    if (filename.size() > 0) {
        string ret = single(filename);
        if (ret.size() > 0) {
            printf("result: %s\n", ret.c_str());
        } else {
            printf("Not recognized...\n");
        }
    } else if (folder.size() > 0) {
        //doit_folder(folder);
        printf("TODO...\n");
    } else {
        help();
    }
}
