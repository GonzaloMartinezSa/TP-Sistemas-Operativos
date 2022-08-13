#ifndef TIEMPO_MAIN_H_
#define TIEMPO_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "utils.h"


long currentTimeMillis();
long timeDifMillis(long start, long end);

#endif
