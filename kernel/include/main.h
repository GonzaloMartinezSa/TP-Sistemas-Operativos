#ifndef MOD2_MAIN_H_
#define MOD2_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <commons/log.h>
#include "comunicacion.h"
#include "../../shared/include/sockets.h"
#include "../include/list.h"
#include "../include/init.h"
#include "../include/planificador.h"
#include "utils.h"


// Elementos planificadorMedianoPlazo
t_list*           lista_block;
t_list*           lista_te;
pthread_mutex_t   prioridad_sr;
int               sr_count;
pthread_mutex_t   m_ejec;

void cerrar_programa();

#endif
