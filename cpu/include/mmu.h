#ifndef MMU_H_
#define MMU_H_

#include <stdio.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <math.h>

#include "utils.h"

extern t_cpuConfig config_cpu;
extern int conexionMemoria;

bool solicitar_info_memoria(t_conf_memoria* conf);

int obtenerDirrecionFisica(int dirLogica, int tp_nivel1, t_log* logger_mmu);
int accesoAMemUsr(int dir, int op, int arg, t_log* logger_mmu);

#endif