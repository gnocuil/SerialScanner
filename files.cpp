#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <dirent.h>
#include <algorithm>

#include "images.h"

using namespace std;

vector<string> single(string filename)
{
    printf("single %s\n", filename.c_str());
    Image img(filename);
    //vector<string> ret = img.search(160);
    vector<string> ret = img.search(120);
    //img.search(140);
    
    for (int i = 0; i < ret.size(); ++i)
        cout << ret[i] << endl;
    
    cout << "finished single\n" << endl;
    
    return ret;
}

int multiple(string dirname)
{
    string sn = dirname + "/_sn_.csv";
    FILE *fout = fopen(sn.c_str(), "w");
    //ofstream fout(sn);

    printf("check for dir %s\n", dirname.c_str());
    vector<string> vs;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirname.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            //printf ("%s\n", ent->d_name);
            string file(ent->d_name);
            if (file.size()>4) {
                string sf = file.substr(file.size()-3);
                if (sf=="jpg" || sf=="JPG" || sf=="png" || sf=="PNG")
                    vs.push_back(file);
            }
        }
        closedir (dir);
        sort(vs.begin(), vs.end());
        int cnt = 0;
        for (int i = 0; i < vs.size(); ++i) {
            //if (i > 3) break;
            string file = dirname+'/'+vs[i];
            vector<string> ret = single(file);
            if (ret.size()>0) {
                printf("Recognized %d numbers in file %s\n", (int)ret.size(), file.c_str());
                for (int j = 0; j < ret.size(); ++j)
                    fprintf(fout, "%d,%s,%s\n", ++cnt, ret[j].c_str(), vs[i].c_str());
            } else {
                //fprintf(fout, "%d,,,,%s\n", i+1, vs[i].c_str());
                printf("Not recognized in file %s\n", file.c_str());
            }
        }
    } else {
        printf("Error reading dir %s\n", dirname.c_str());
    }

    
    fclose(fout);
}
