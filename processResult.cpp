#include <stdio.h>
#include <cstring>
#include <string>
#include <iostream>
#include <map>
#include <vector>

using namespace std;



int main(int argc, char**argv)
{
    puts(argv[1]);
    freopen(argv[1], "r", stdin);
    
    map<string, int> mp;
    
    char line[1000];
    int repeat = 0;
    int tot = 0;
    int invalid777 = 0;
    int invalidlll = 0;
    
    vector<string> number;
    vector<string> path;
    
    vector<string> number_r;
    vector<string> path_r;

    while (gets(line)) {
        int len = strlen(line);
        string id;
        string num1;
        string num2;
        string file;
        int cnt = 0;
        for (int i = 0; i < len; ++i) {
            if (cnt == 0) {
                if (line[i] == ',') {
                    ++cnt;
                } else {
                    id+=line[i];
                }
            } else if (cnt == 1) {
                if (line[i] == ',') {
                    ++cnt;
                } else {
                    num1+=line[i];
                }
            } else if (cnt == 2) {
                if (line[i] == ',') {
                    ++cnt;
                } else {
                    num2+=line[i];
                }
            } else {
                file+=line[i];
            }
        }
        string num = num1 + ',' + num2;
        if (num.find("77777") != string::npos) {
            invalid777++;
            continue;
        }
        if (num.find("lllll") != string::npos) {
            invalidlll++;
            continue;
        }
        if (mp.count(num) > 0) {
            repeat++;
            //printf("repeat: %s %s | %s\n", num.c_str(), path[mp[num]].c_str(), file.c_str());
            number_r.push_back(num);
            number_r.push_back(num);
            path_r.push_back(path[mp[num]]);
            path_r.push_back(file);
            continue;
        }
        mp[num] = tot;
        ++tot;
        number.push_back(num);
        path.push_back(file);
    }
    printf("Tot valid=%d\n", tot);
    printf("repeat=%d\n", repeat);
    printf("invalid (77777) = %d\n", invalid777);
    printf("invalid (lllll) = %d\n", invalidlll);
    
    FILE *f1 = fopen("results.csv", "w");
    for (int i = 0; i < tot; ++i)
        fprintf(f1, "%d,%s,%s\n", i+1, number[i].c_str(), path[i].c_str());
    fclose(f1);

    FILE *f2 = fopen("repeats.csv", "w");
    for (int i = 0; i < number_r.size(); ++i)
        fprintf(f2, "%d,%s,%s\n", i+1, number_r[i].c_str(), path_r[i].c_str());
    fclose(f2);    
    
}
