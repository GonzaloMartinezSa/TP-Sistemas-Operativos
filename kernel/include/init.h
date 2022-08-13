#ifndef INIT_H_
#define INIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "comunicacion.h"
#include "../include/list.h"
#include "../include/planificador.h"
#include "utils.h"

t_kernelConfig leer_config(char* argv);
t_planificador  iniciar_planificador(algoritmo modo);
bool            iniciar_kernel();
void            test_list(t_log* logger, t_planificador* planificador);
static void     atender_consolas(void* void_args);
void            imprimir_listas_estados(t_planificador* planificador);

extern t_list*           lista_block;
extern pthread_mutex_t   prioridad_sr;
extern int               sr_count;
extern pthread_mutex_t   m_ejec;
extern t_list*           lista_te; 

int conexionCPU_dispatch;
int conexionCPU_interrupt;
int conexionMemoria;

void imprimir_listas_estados(t_planificador* planificador);
void test_list(t_log* logger, t_planificador* planificador);


#endif /* INIT_H_ */
