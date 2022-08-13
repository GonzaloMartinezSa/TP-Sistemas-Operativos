#ifndef UITLS_MEMORY_H_
#define UITLS_MEMORY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include <sys/mman.h>
#include <semaphore.h>

typedef enum {
    CLOCK, 
    CLOCKM,
} algoritmoReemplazo;

typedef enum {
    OPM_READ,
    OPM_WRITE,
} OPM;

typedef struct {
    uint32_t* datos;
} t_marco;

typedef struct {
    int  marco;
    bool P;
    bool U;
    bool M;
    // Numero de pagina del proceso
} t_descpaginaSegundoNivel;

typedef struct {
    bool enUso;
    t_descpaginaSegundoNivel*(*entradas);
} t_tablaPaginaSegundoNivel;

typedef struct {
    bool enUso;
    int* entradas;     // Numero de TP2 ABSOLUTO
    int  cant_entradas;
    int  primer_marco; // El primer marco que tiene asociado el proceso
    int  punteroClock;
    char swap_filename[50];
} t_tablaPaginaPrimerNivel;

typedef struct {
    char puerto_escucha [10];
	int  tam_memoria;
    int  tam_pagina;
	int  entradas_por_tabla;
	int  retardo_por_memoria;
    algoritmoReemplazo algoritmo_reemplazo;
    int  marcos_por_proceso;
	int  retardo_swap;
    char path_swap[50]; 
} t_m_config;

typedef struct {
    int fd;
    char* server_name;
} t_procesar_conexion_args;

typedef struct {
    bool enUso;
    int  NTP1;
    int  ETP1; // Numero de TP2 RELATIVO
    int  ETP2;
} t_bitmap_marco;


t_log*     logger;
t_m_config conf;

#define MAX_TABLAS_P1 10
#define MAX_TABLAS_P2 160
t_tablaPaginaPrimerNivel  listaTablasPrimerNivel[MAX_TABLAS_P1];
t_tablaPaginaSegundoNivel listaTablasSegundoNivel[MAX_TABLAS_P2];

int asignadosPrimerNivel;
int asignadosSegundoNivel;

int TOTAL_MARCOS;
int MEM_MAX_X_PROCESO;
int CANT_UINTS_MARCO;
bool* indice_marco_uso;

void* espacio_usuario;
t_bitmap_marco * bitmap_marco;

sem_t sem;

#endif /* INIT_H_ */