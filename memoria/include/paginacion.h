#ifndef PAG_MEMORY_H_
#define PAG_MEMORY_H_

#include "utils.h"
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
#include "reemplazo.h"

int inicializarProceso(int tamano, int pid) ;
bool finalizarProceso(int indiceP1);
t_marco extraer_pagina_swap(int numeroTabla, int entrada, int NTP1);
int  buscar_marco_libre(int NTP1);
bool llevar_marco_a_swap(int numero_marco);
bool suspenderProceso(int NTP1);
int acceso_tabla(int arg1, int arg2, int arg3);

#endif /* INIT_H_ */