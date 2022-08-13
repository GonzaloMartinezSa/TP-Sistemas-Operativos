#ifndef INIT_MEMORY_H_
#define INIT_MEMORY_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../include/utils.h"
#include "../include/comunicaciones.h"

void leerConfig(char* argv);
bool iniciar_estructuras_memoria();

#endif /* INIT_H_ */