#ifndef KERNEL_UTILS_H_
#define KERNEL_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "../../shared/include/protocolo.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

typedef enum{
    FIFO,
    SRT,
}algoritmo;

typedef struct kernelConfig{
    char      ip_memoria[50];
    char      puerto_memoria[10];
    char      ip_cpu[50];
    char      puerto_cpu_dispatch[10];
    char      puerto_cpu_interrupt[10];
    char      puerto[10];
    algoritmo algoritmo;
    int       estimacion_inicial;
    double    alfa;
    int       multi_prog;
    int       tmax;
}t_kernelConfig;

typedef struct l_nodo{
    struct l_nodo*  ant;
    struct l_nodo*  sgte;
    t_pcb*          pcb;
}t_nodo;

typedef struct l_desc{
    char    nombre[50];
    int     cantidad;
    t_nodo* first;
    t_nodo* last;
}t_desc;

typedef enum{
    ENEW,
    EREADY,
    EEXEC,
    EBLOCK,
    ESB,
    ESR,
    EEXIT,
}e_status;

typedef struct planificador{
    // Algoritmo de planificacion
    algoritmo        modo;
    // Listas de planificacion.. proteccion de concurrencia y contadores de elementos
    t_desc*          list [7];
    pthread_mutex_t  mutex[7];
    sem_t            sem  [7];
    // Contadores y proteccion para numeracin de procesos interna
    pthread_mutex_t  pid_mutex;
    int              pid;
    // Lista que relaciona PID y socket
    t_list*          lista;
    // Contador de procesos en planificadores
    sem_t            multi_prog;
}t_planificador;

typedef struct {
    t_log* log;
    t_kernelConfig   config;
    t_planificador * planner;
} t_procesar_consolas_args;

typedef struct
{
	int  id;
	int  socket;
	long inicio;
}t_socketID;


typedef enum{
    BLOQUEADO,
    SUSPENDIDO,
    LISTO,
}b_status;

typedef struct l_nodo_b{
    t_pcb*      pcb;
    pthread_t   th;
    int         block_t;
    b_status    status;
}t_nodo_block;

typedef struct{
    int      pid;
    long     elapsed;
}t_tiempo_ac;


typedef struct {
    int fd;
    char* server_name;
} t_procesar_conexion_args;

t_log*          logger;
t_kernelConfig  config;
t_planificador  planificador;

pthread_mutex_t  comunicacion_memoria;

#endif