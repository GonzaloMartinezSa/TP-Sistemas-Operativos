#ifndef CICLO_DE_INSTRUCCION_H_
#define CICLO_DE_INSTRUCCION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../../shared/include/protocolo.h"
#include "utils.h"
#include "mmu.h"
#include "tlb.h"

void ciclo_de_instruccion(t_pcb*);

extern int conexionKernel_dispatch;
//extern int conexionKernel_interrupt;
extern t_log* logger_cpu_ciclo;

extern bool hay_interrupcion; //de manejoServidor.h
extern int kernel_com;        //de main.h
extern t_cpuConfig config_cpu;

#endif