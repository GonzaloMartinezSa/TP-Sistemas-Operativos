#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../../shared/include/protocolo.h"
#include "../../shared/include/sockets.h"

typedef struct consoleConfig{
    char* ip;
    char* port;
    pid_t id;
}consoleConfig;


t_config* leer_config(void);
t_instruccion* leer_programa(t_log* logger, char* filename, int* inst);

#endif /* UTILS_H_ */
