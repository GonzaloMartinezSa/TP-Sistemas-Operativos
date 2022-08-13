#ifndef CPU_MAIN_H_
#define CPU_MAIN_H_

#include <stdio.h>
#include <pthread.h>
#include <commons/log.h>
#include "utils.h"
#include "init.h"
#include "comunicaciones.h"
#include "cicloInstruccion.h"
#include "tlb.h"

int conexionKernel_dispatch;
int conexionKernel_interrupt;
int conexionMemoria;
t_log* logger_cpu_ciclo;
t_cpuConfig config_cpu;
int estoy_ejecutando;

#endif