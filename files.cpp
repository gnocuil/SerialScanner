#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <dirent.h>
#include <algorithm>

#include "images.h"

using namespace std;

int debug;

static int rid = 0;
static FILE *fout_root = NULL;

void printRoot(const vector<string>& ret, string filename)
{
    //printf("##########printRoot############# %d\n", ret.size());
    
    for (int i = 0; i < ret.size(); ++i) {
        ++rid;
        fprintf(fout_root, "%d,%s,%s\n", rid, ret[i].c_str(), filename.c_str());
    }
    fflush(fout_root);
}

vector<string> single(string filename, int threshold = -1)
{
    //printf("single %s\n", filename.c_str());
    Image img(filename);
    img.debug = debug;
    vector<string> ret;
    if (threshold >= 0) {
        ret = img.search(threshold);
    } else {
        int ths[] = {160, 100, 130, 180};
        int n = sizeof(ths)/sizeof(int);
        
        for (int i = 0; i < n; ++i) {
            vector<string> cur = img.search(ths[i]);
            for (int j = 0; j < cur.size(); ++j)
                ret.push_back(cur[j]);
            //if (ret.size()>0) break;
        }
    }
    //vector<string> ret = img.search(100);
    //160~100
    //img.search(140);
    
    cout << "------------" << filename << "------------" << endl;
    for (int i = 0; i < ret.size(); ++i)
        cout << ret[i] << endl;
    cout << "------------" << "------------" << endl;
    //cout << "finished single\n" << endl;
    
    return ret;
}

static vector<string> _multiple(string dirname, int depth)
{
    vector<string> ret;
    if (depth == 0) return ret;
    
    string sn = dirname + "/_sn_.csv";
    
    //ofstream fout(sn);

    printf("check for dir %s\n", dirname.c_str());
    vector<string> vs;
    vector<string> dirs;
    DIR *dir;
    struct dirent *ent;
    bool ignore = false;
    if ((dir = opendir (dirname.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            //printf ("%s\n", ent->d_name);
            string file(ent->d_name);
            if (ent->d_type == DT_DIR) {
                if (file[0] != '.' or file.size()>=3)
                    dirs.push_back(file);
            } else if (file.size()>4) {
                if (file == "ignore") ignore = true;
                string sf = file.substr(file.size()-3);
                if (sf=="jpg" || sf=="JPG" || sf=="jpeg" || sf=="JPEG" || sf=="png" || sf=="PNG")
                    vs.push_back(file);
            }
        }
        closedir (dir);
        if (ignore) {
            printf("Ignore current dir:%s\n", dirname.c_str());
        } else {
            sort(vs.begin(), vs.end());
            int cnt = 0;
            if (vs.size() > 0) {
                FILE *fout = fopen(sn.c_str(), "w");
                for (int i = 0; i < vs.size(); ++i) {
                    //if (i > 3) break;
                    string file = dirname+'/'+vs[i];
                    vector<string> cur_ret = single(file);
                    if (cur_ret.size()>0) printRoot(cur_ret, file);
                    if (cur_ret.size()>0) {
                        printf("Recognized %d numbers in file %s\n", (int)cur_ret.size(), file.c_str());
                        for (int j = 0; j < cur_ret.size(); ++j) {
                            //cout<<cur_ret[j]<<endl;
                            ret.push_back(cur_ret[j] + "," + file);
                            fprintf(fout, "%d,%s,%s\n", ++cnt, cur_ret[j].c_str(), file.c_str());
                        }
                    } else {
                        //fprintf(fout, "%d,,,,%s\n", i+1, vs[i].c_str());
                        printf("Not recognized in file %s\n", file.c_str());
                        fprintf(fout, ",,,%s\n", file.c_str());
                    }
                }
                fclose(fout);
            }
        }
        
        //printf("ready for dir...\n");
        if (dirs.size() > 0) {
            for (int i = 0; i < dirs.size(); ++i) {
                string next = dirname+'/'+dirs[i];
                vector<string> nxtret = _multiple(next, depth-1);
                for (int j = 0; j < nxtret.size(); ++j)
                    ret.push_back(nxtret[j]);
            }
        }
    } else {
        printf("Error reading dir %s\n", dirname.c_str());
    }
    return ret;
}

int multiple(string dirname, int depth)
{
    fout_root = fopen("_sn_all_.csv", "w");
    vector<string> ret = _multiple(dirname, depth);
    
    //for (int i = 0; i < ret.size(); ++i)
    //    fprintf(fout, "%d,%s\n", i+1, ret[i].c_str());
    fclose(fout_root);
    return ret.size();
}
