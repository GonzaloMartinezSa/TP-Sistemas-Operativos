#ifndef REEMPLAZO_MEMORY_H_
#define REEMPLAZO_MEMORY_H_

#include <commons/log.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>
#include "memusr.h"
#include "utils.h"

int reemplazoClockMejorado(int numeroTablaPrimerNivel);
int reemplazoClock(int numeroTablaPrimerNivel);

#endif /* INIT_H_ */