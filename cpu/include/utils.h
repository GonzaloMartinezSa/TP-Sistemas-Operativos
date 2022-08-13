#ifndef UITLS_CPU_H_
#define UITLS_CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include <pthread.h>

typedef enum{
    FIFO,
    LRU,
}algoritmo_reemplazo;

typedef struct cpuConfig{
    int   entradas_tlb;
    algoritmo_reemplazo reemplazo_tlb;
    int   retardo_noop;
    char ip_memoria[80];
    char puerto_memoria[15];
    char puerto_escucha_dispatch[15];
    char puerto_escucha_interrupt[15];
    pthread_mutex_t inst_mutex;
}t_cpuConfig;

typedef struct {
    t_log* log;
    t_cpuConfig  config;
    t_pcb* pcb;
    int*   kernel_com;
} t_pcbs_args;

typedef struct {
    int cant_entradas_x_pagina;
    int tamano_pagina;
} t_conf_memoria;


t_conf_memoria c_memoria;
t_pcb* pcb;

#endif /* INIT_H_ */