#include "../include/tiempo.h"

long currentTimeMillis() {
  struct timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec * 1000 + time.tv_usec / 1000;
}

long timeDifMillis(long start, long end){
    return end - start;
}