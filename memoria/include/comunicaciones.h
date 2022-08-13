#ifndef COMUNICACIONES_MEMORY_H_
#define COMUNICACIONES_MEMORY_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "utils.h"
#include "memusr.h"
#include "paginacion.h"
#include "reemplazo.h"

int procesar_conexion(int* socket);

int conexionCPU;
int conexionKernel;

extern sem_t sem;

#endif /* INIT_H_ */