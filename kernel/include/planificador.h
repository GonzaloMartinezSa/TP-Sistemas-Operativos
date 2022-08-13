#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../include/list.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "comunicacion.h"
#include "utils.h"

extern t_list*           lista_block;
extern pthread_mutex_t   prioridad_sr;
extern int               sr_count;
extern pthread_mutex_t   m_ejec;

void planificador_largoplazo(void* void_args) ;
void planificador_medianoplazo(void* void_args);
void insertar_en_ready(t_pcb* pcb, bool reinsert);
void manejo_bloqueo(void* void_args);
void manejo_exit(void* void_args);

#endif
