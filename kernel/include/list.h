#ifndef LIST2_H_
#define LIST2_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "utils.h"
#include <commons/collections/list.h>

t_desc*    lista_crear(char* nombre);
bool       lista_insertar(t_log* logger, t_desc *desc, t_pcb* pcb, algoritmo modo, bool prioridad);
t_pcb*     lista_extraer(t_desc* desc);
t_pcb*     crear_pcb(uint16_t id, uint16_t tamano, uint8_t pc, int tp, double srt, t_instruccion* programa); 
t_pcb*     lista_buscar_extraer(t_desc* desc, uint16_t id);
void       agregarSocketID(t_list *lista, t_socketID *socketID);
t_socketID borrarSocketID(int pcb_id, t_list* lista);
void       lista_eliminar(t_desc* prt);
void       imprimir_lista(t_desc* desc);
#endif /* LIST_H_ */



