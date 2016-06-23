#pragma once

#include <cstdio>
#include <string>
#include <sys/time.h>

class Timer {
public:
    Timer() {
        gettimeofday(&tv, NULL);
    }
    
    void current(std::string msg) {
        struct timeval tv2;
        gettimeofday(&tv2, NULL);
        double current_time = (tv2.tv_sec - tv.tv_sec)*1000 + (tv2.tv_usec - tv.tv_usec) / 1000.0;
        printf("*==TIME=%.2lfms (%s)\n", current_time, msg.c_str());
    }
    
    void reset() {
        gettimeofday(&tv, NULL);
    }
    
    
private:
    struct timeval tv;
};
