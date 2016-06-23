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
    system("rm -f debug/*");
    string filename = "";
    string folder = "";
    int threshold = -1;
    int depth = 100;
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
        } else if (strcmp(argv[i],"-t")==0 && i+1<argc) {
            ++i;
            sscanf(argv[i], "%d", &threshold);
        } else if (strcmp(argv[i],"-d")==0 && i+1<argc) {
            ++i;
            sscanf(argv[i], "%d", &depth);
        } else {
            help();
        }
    }
    
    if (filename.size() > 0) {
        vector<string> ret = single(filename, threshold);
        if (ret.size() > 0) {
            printf("result numbers: %d\n", (int)ret.size());
        } else {
            printf("Not recognized...\n");
        }
    } else if (folder.size() > 0) {
        int ret = multiple(folder, depth);
        if (ret > 0) {
            printf("result numbers: %d\n", ret);
        } else {
            printf("Not recognized...\n");
        }
    } else {
        help();
    }
}
