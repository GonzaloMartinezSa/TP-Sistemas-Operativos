#ifndef COMUNICACION_MOD2_H_
#define COMUNICACION_MOD2_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../include/init.h"
#include "../include/list.h"
#include "tiempo.h"
#include "utils.h"

int     server_escuchar(char* server_name, int server_socket); 
sc_cpu  enviar_PCB(t_pcb* pcb, uint32_t * tiempoBloqueo_);
void    enviar_interrupt(IR_code ir);
int     notificar_memoria(sc_rmem sc, int arg1 , int arg2, int arg3);

extern int conexionCPU_dispatch;
extern int conexionCPU_interrupt;
extern int conexionMemoria;

extern t_list* lista_te;

#endif
