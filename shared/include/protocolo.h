
#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include <inttypes.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef enum
{
	NOOP,
	IO,
	READ,
	COPY,
	WRITE,
	EXIT,
    HEADER,
	PROGRAMA,
	KERNELPCB,
	CPUPCB,
    INSTRUCCION,
	TP,
	CPURTA,
	INTERRUPCION,
	VALIDACION,
}op;

typedef struct
{
	int coop;
	int arg1;
	int arg2;

} t_instruccion;

typedef struct pcb{
    uint16_t  id;
    uint16_t  tamano;
    uint8_t   pc;
    int       tp;		
    float     srt;
    t_instruccion* programa;
}t_pcb;


typedef enum
{
	FINALIZADO,
	BLOCK,
	IR,
	ERROR,
}sc_cpu;

typedef enum
{
	DESALOJO,
}IR_code;


typedef struct{
	int   sc;
    long  tiempoBloqueo;
}t_rta_consola;

typedef enum
{
	MEMINFO,
	INICIALIZAR,
	SUSPENDERP,
	LIBERAR,
	ACCESO_A_TABLA,
	ACCESO_USUARIO,
	SOK,
	SERROR,
}sc_rmem;


typedef enum
{
	VALIDADO,
	VAL_ERROR,
	REINICIAR_COM,
}sc_val;

bool enviarPrograma (int inst, t_instruccion* instrucciones, int memoria, int conexion);

bool recv_header(int fd, int* instrucciones, int* memoria);


bool recv_header_memoria(int fd, uint8_t* instrucciones, pid_t* id, int* memoria);
bool send_header_memoria(int fd, uint8_t instrucciones, pid_t id, int memoria);

bool recv_instruccion(int fd, op* co_op, int* arg1, int* arg2);
bool send_instruccion(int fd, op co_op, int arg1, int arg2);

bool recv_tp(int fd, int* tp);
bool send_tp(int fd, int tp);

bool send_liberar(int fd, int pid, int tp);
bool recv_liberar(int fd, int* pid, int* tp); 

int  tamano_intrucciones_pcb(t_pcb* pcb);
bool send_pcb(int socket, t_pcb* pcb, sc_cpu* statusCode, uint32_t* tiempoBloqueo);

//bool send_header_PCB_CTK(int fd, t_pcb* pcb, sc_cpu *statusCode, uint32_t *tiempoBloqueo);
bool recv_header_PCB_CTK(int fd, t_pcb* pcb, uint8_t *tamanoInstrucciones, sc_cpu *statusCode, uint32_t *tiempoBloqueo);
void* serializar_header_PCB_KTC(t_pcb* pcb, void* stream, uint8_t* tamanoInstrucciones);
void* serializar_header_PCB_CTK(t_pcb* pcb, sc_cpu *statusCode, uint32_t *tiempoBloqueo, void* stream, uint8_t* tamano);

//bool send_header_PCB_KTC(int fd, t_pcb* pcb);
bool recv_header_PCB_KTC(int fd, t_pcb* pcb, uint8_t *tamanoInstrucciones);

sc_cpu enviar_pcb(t_pcb* pcb);

bool recv_rta_kernel (int fd, t_rta_consola* msj);
bool send_rta_consola(int socket, t_rta_consola* msj);

bool send_suspenderProceso(int fd, int pid, int tp);
bool recv_suspenderProceso(int fd, int* pid, int* tp);


bool send_protocoloMemoria(int fd, sc_rmem co, int arg1, int arg2, int arg3);
bool recv_protocoloMemoria(int fd, int* arg1, int* arg2, int* arg3);


bool send_validacion(int fd);
bool recv_validacion(int fd,sc_val* sc);
#endif