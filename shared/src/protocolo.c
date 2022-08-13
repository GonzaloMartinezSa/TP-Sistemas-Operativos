
#include "../include/protocolo.h"

// Header

static void* serializar_programa(void* stream, int memoria, t_instruccion* programa, int inst){
    int offset   = 0;

    op cop = PROGRAMA;

    memcpy(stream, &cop, sizeof(op));
    offset += sizeof(op);
    memcpy(stream + offset, &inst, sizeof(int));
    offset += sizeof(int);
    memcpy(stream + offset, &memoria, sizeof(int));
    offset += sizeof(int);

    for(int i = 0; i < inst; i ++){
        memcpy(stream + offset, &(programa[i].coop), sizeof(op) );
        offset += sizeof(op);
        memcpy(stream + offset, &(programa[i].arg1), sizeof(int) );
        offset += sizeof(int);
        memcpy(stream + offset, &(programa[i].arg2), sizeof(int) );
        offset += sizeof(int);
    }
}


bool enviarPrograma(int inst, t_instruccion* instrucciones, int memoria, int conexion) {

        size_t headerSize = sizeof(op) + sizeof(int) + sizeof(int);
        size_t instSize = sizeof(t_instruccion) * inst;
        // Envio las instrucciones
        void* stream = malloc(headerSize + instSize);
        serializar_programa(stream, memoria, instrucciones, inst);

        send(conexion, stream, headerSize + instSize, 0);
        free(stream);
}

static void deserializar_header(void* stream, int* instrucciones,  int* memoria) {
    memcpy(instrucciones, stream, sizeof(int));
    memcpy(memoria, stream+sizeof(int) , sizeof(int));
}


static void deserializar_header_memoria(void* stream, uint8_t* instrucciones, pid_t* id, int* memoria) {
    memcpy(id, stream, sizeof(pid_t));
    memcpy(instrucciones, stream + sizeof(pid_t), sizeof(uint8_t));
    memcpy(memoria, stream+sizeof(uint8_t) , sizeof(int));
}

bool send_header_memoria(int fd, uint8_t instrucciones, pid_t id, int memoria) {
    size_t size  = sizeof(op) + sizeof(uint8_t) + sizeof(pid_t) + sizeof(int);
    void* stream = malloc(size);
    int offset = 0;

    op cop = HEADER;

    
    memcpy(stream , &cop, sizeof(op));
    offset += sizeof(op);
    memcpy(stream + offset, &id , sizeof(pid_t));
    offset += sizeof(pid_t);
    memcpy(stream + offset, &instrucciones, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(stream + offset, &memoria, sizeof(int));

    

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_header_memoria(int fd, uint8_t* instrucciones, pid_t* id, int* memoria) {

    size_t size = sizeof(uint8_t) + sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }

    deserializar_header_memoria(stream, instrucciones, id, memoria);

    free(stream);
    return true;
}

bool recv_header(int fd, int* instrucciones, int* memoria) {

    size_t size = sizeof(int) * 2;
    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        printf("recibio mal el programa de la consola\n");
        free(stream);
        return false;
    }

    deserializar_header(stream, instrucciones, memoria);

    free(stream);
    return true;
}


// Instruccion
static void* serializar_instruccion(op co_op, int arg1, int arg2) {
    void* stream = malloc(sizeof(op) * 2 + sizeof(int) * 2);
    void* pos    = stream;

    op cop = INSTRUCCION;

    //Aviso del protocolo de com
    memcpy(pos, &cop, sizeof(op));
    pos += sizeof(op);

    //Codigo de instruccion
    memcpy(pos, &co_op, sizeof(op));
    pos += sizeof(op);

    memcpy(pos, &arg1, sizeof(int));
    pos += sizeof(int);

    memcpy(pos, &arg2, sizeof(int));
    pos += sizeof(int);    

    return stream;
}

static void deserializar_instruccion(void* stream,op* co_op, int* arg1, int* arg2) {
    memcpy(co_op, stream, sizeof(op));
    memcpy(arg1, stream+sizeof(op), sizeof(int));
    memcpy(arg2, stream+sizeof(op)+sizeof(int), sizeof(int));
}

bool send_instruccion(int fd, op co_op, int arg1, int arg2) {
    size_t size  = sizeof(op) * 2 + sizeof(int) *2;

    void* stream = serializar_instruccion(co_op, arg1, arg2);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_instruccion(int fd, op* co_op, int* arg1, int* arg2) {

    size_t size  = sizeof(op) + sizeof(int) *2;
    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }
    deserializar_instruccion(stream, co_op, arg1, arg2);
    free(stream);
    return true;
}

int tamano_intrucciones_pcb(t_pcb* pcb){

    t_instruccion* instrucciones = pcb->programa;

    int size = 0;

    if(instrucciones == NULL)
        return 0;

    while(1){
        if(instrucciones[size].coop == EXIT){
            size++;
            break;
        }
        else
            size++;
    }
    return size;
}

// TP
static void* serializar_tp(int tp) {
    void* stream = malloc(sizeof(int));
    void* pos    = stream;
    memcpy(pos, &tp, sizeof(int));
    return stream;
}

static void deserializar_tp(void* stream, int* tp) {
    memcpy(tp, stream, sizeof(int));
}

bool send_tp(int fd, int tp) {
    size_t size  = sizeof(int);

    void* stream = serializar_tp(tp);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_tp(int fd, int* tp) {

    size_t size  = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }
    deserializar_tp(stream, tp);
    free(stream);
    return true;
}

// Liberar memoria
static void* serializar_liberar(int pid, int tp) {
    void* stream = malloc(sizeof(op) + sizeof(int) *2);
    void* pos    = stream;
    op cop = LIBERAR;
    memcpy(pos, &cop, sizeof(op));
    pos += sizeof(op);
    memcpy(pos, &pid, sizeof(int));
    pos += sizeof(int);
    memcpy(pos, &tp, sizeof(pid_t));
    return stream;
}

static void deserializar_liberar(void* stream, int* pid, int* tp) {
    memcpy(pid, stream, sizeof(int));
    memcpy(tp , stream+sizeof(int), sizeof(int));
}

bool send_liberar(int fd, int pid, int tp) {
    size_t size  = sizeof(op) + sizeof(int) * 2;

    void* stream = serializar_liberar(pid, tp);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_liberar(int fd, int* pid, int* tp) {

    size_t size = sizeof(int) * 2;
    void* stream = malloc(size);
    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }
    deserializar_liberar(stream, pid, tp);
    free(stream);
    return true;
}

//

// Envio el pcb
bool send_pcb(int socket, t_pcb* pcb, sc_cpu* statusCode, uint32_t* tiempoBloqueo){

    uint8_t inst = tamano_intrucciones_pcb(pcb);
	size_t size  = sizeof(op) + sizeof(sc_cpu) + sizeof(int) + sizeof(uint16_t) * 2 + sizeof(uint8_t) * 2 + sizeof(double) + sizeof(uint32_t);
    void* stream = malloc(size);
    
    stream = serializar_header_PCB_CTK(pcb, statusCode, tiempoBloqueo, stream, &inst);


    if(send(socket, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    // Aca deberia verificar la recepcion del header..
    sc_val sc;
    recv_validacion(socket , &sc);

    free(stream);
    
    return true;
}

// Envio de PCB de CPU a Kernel
void* serializar_header_PCB_CTK(t_pcb* pcb, sc_cpu *statusCode, uint32_t *tiempoBloqueo, void* stream, uint8_t* tamanoInstrucciones) {
    size_t size  = sizeof(op) + sizeof(sc_cpu) + sizeof(int) + sizeof(uint16_t) * 2 + sizeof(uint8_t) * 2 + sizeof(double) + sizeof(uint32_t);
    void* pos    = stream;

    op cop = CPUPCB;

    memcpy(pos, &cop, sizeof(op));
    pos += sizeof(op);
    memcpy(pos, statusCode, sizeof(sc_cpu));
    pos += sizeof(sc_cpu);
    memcpy(pos, tiempoBloqueo, sizeof(uint32_t));
    pos += sizeof(uint32_t);
    memcpy(pos, &pcb->id, sizeof(uint16_t));
    pos += sizeof(uint16_t);
    memcpy(pos, &pcb->tamano, sizeof(uint16_t));
    pos += sizeof(uint16_t);
    memcpy(pos, &pcb->pc, sizeof(uint8_t));
    pos += sizeof(uint8_t);
    memcpy(pos, &pcb->tp, sizeof(int));
    pos += sizeof(int);
    memcpy(pos, &pcb->srt, sizeof(double));
    pos += sizeof(double);
    memcpy(pos, tamanoInstrucciones, sizeof(uint8_t));

    return stream;
}
/*
bool send_header_PCB_CTK(int fd, t_pcb* pcb, sc_cpu *statusCode, uint32_t *tiempoBloqueo) {
    size_t size  = sizeof(op) + sizeof(sc_cpu) + sizeof(int) + sizeof(uint16_t) * 2 + sizeof(uint8_t) * 2 + sizeof(double) + sizeof(uint32_t);

    void* stream = serializar_header_PCB_CTK(pcb, statusCode, tiempoBloqueo);
    
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}
*/

static void deserializar_header_PCB_CTK(void* stream, t_pcb* pcb, uint8_t *tamanoInstrucciones, sc_cpu *statusCode, uint32_t *tiempoBloqueo){
    int offset = 0;
    op basura = 0;

    memcpy(statusCode, stream + offset, sizeof(sc_cpu));
    offset += sizeof(sc_cpu);
    memcpy(tiempoBloqueo, stream+offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(pcb->id), stream+offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(&(pcb->tamano), stream+offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(&(pcb->pc), stream+offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&(pcb->tp), stream+offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pcb->srt), stream+offset, sizeof(double)); 
    offset += sizeof(double);
    memcpy(tamanoInstrucciones, stream+offset, sizeof(uint8_t));

}

bool recv_header_PCB_CTK(int fd, t_pcb* pcb, uint8_t *tamanoInstrucciones, sc_cpu *statusCode, uint32_t *tiempoBloqueo) {
    
    size_t size =  sizeof(sc_cpu) + sizeof(int) + sizeof(uint16_t) * 2 + sizeof(uint8_t) * 2 + sizeof(double) + sizeof(uint32_t);
    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }
    deserializar_header_PCB_CTK(stream, pcb, tamanoInstrucciones, statusCode, tiempoBloqueo);
    
    //*statusCode = 2;

    //*tiempoBloqueo = 5;
    free(stream);
    return true;
}
//

// Enviar PCB de Kernel a CPU
void* serializar_header_PCB_KTC(t_pcb* pcb, void* stream, uint8_t* tamanoInstrucciones) {
    void* pos    = stream;

    op cop = KERNELPCB;

    memcpy(pos, &cop, sizeof(op));
    pos += sizeof(op);
    memcpy(pos, &pcb->id, sizeof(uint16_t));
    pos += sizeof(uint16_t);
    memcpy(pos, &pcb->tamano, sizeof(uint16_t));
    pos += sizeof(uint16_t);
    memcpy(pos, &pcb->pc, sizeof(uint8_t));
    pos += sizeof(uint8_t);
    memcpy(pos, &pcb->tp, sizeof(int));
    pos += sizeof(int);
    memcpy(pos, &pcb->srt, sizeof(double));
    pos += sizeof(double);
    memcpy(pos, tamanoInstrucciones, sizeof(uint8_t));

    
    return stream;
}

/*
bool send_header_PCB_KTC(int fd, t_pcb* pcb) {
    size_t size  = sizeof(op)  + sizeof(int) + sizeof(uint16_t) * 2 + sizeof(uint8_t) * 2 + sizeof(double);

    void* stream = serializar_header_PCB_KTC(pcb);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}
*/

static void deserializar_header_PCB_KTC(void* stream, t_pcb* pcb, uint8_t *tamanoInstrucciones){
    int offset = 0;
    memcpy(&(pcb->id), stream, sizeof(uint16_t));
    offset = sizeof(uint16_t);
    memcpy(&(pcb->tamano), stream+offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(&(pcb->pc), stream+offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&(pcb->tp), stream+offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pcb->srt), stream+offset, sizeof(double)); 
    offset += sizeof(double);
    memcpy(tamanoInstrucciones, stream+offset, sizeof(uint8_t));

}

bool recv_header_PCB_KTC(int fd, t_pcb* pcb, uint8_t *tamanoInstrucciones) {

    size_t size = sizeof(int) + sizeof(uint16_t) * 2 + sizeof(uint8_t) * 2 + sizeof(double);
    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }

    deserializar_header_PCB_KTC(stream, pcb, tamanoInstrucciones);

    free(stream);
    return true;
}

//


void* serializar_rta_kernel(t_rta_consola* msj) {
    void* stream = malloc(sizeof(int) + sizeof(long));
    memcpy(stream, &msj->sc, sizeof(int));
    memcpy(stream + sizeof(int), &msj->tiempoBloqueo, sizeof(long));
    return stream;
}

// Envio el pcb
bool send_rta_consola(int socket, t_rta_consola* msj){

    size_t size  = sizeof(int) + sizeof(long);

    void* stream = serializar_rta_kernel(msj);
    
    if (send(socket, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);

    return true;
}


void deserializar_rta_kernel(void* stream, t_rta_consola* msj){

    int offset = 0;
    memcpy(&(msj->sc), stream, sizeof(int));
    offset = sizeof(int);
    memcpy(&(msj->tiempoBloqueo), stream+offset, sizeof(long));

}

bool recv_rta_kernel(int fd, t_rta_consola* msj) {

    size_t size  = sizeof(int) + sizeof(long);
    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }

    deserializar_rta_kernel(stream, msj);

    free(stream);
    return true;
}

// Suspender proceso
static void* serializar_suspenderProceso(int pid, int tp) {
    void* stream = malloc(sizeof(op) + sizeof(int) *2);
    void* pos    = stream;
    op cop = SUSPENDERP;
    memcpy(pos, &cop, sizeof(op));
    pos += sizeof(op);
    memcpy(pos, &pid, sizeof(int));
    pos += sizeof(int);
    memcpy(pos, &tp, sizeof(pid_t));
    return stream;
}

static void deserializar_suspenderproceso(void* stream, int* pid, int* tp) {
    memcpy(pid, stream, sizeof(int));
    memcpy(tp , stream+sizeof(int), sizeof(int));
}

bool send_suspenderProceso(int fd, int pid, int tp) {
    size_t size  = sizeof(op) + sizeof(int) * 2;

    void* stream = serializar_suspenderProceso(pid, tp);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_suspenderProceso(int fd, int* pid, int* tp) {

    size_t size = sizeof(int) * 2;
    void* stream = malloc(size);
    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }
    deserializar_suspenderproceso(stream, pid, tp);
    free(stream);
    return true;
}

//


// Protocolo Base de comunicacion con memoria
static void* serializar_protocoloMemoria(sc_rmem sc, int arg1, int arg2, int arg3) {

    void* stream = malloc(sizeof(sc_rmem) + sizeof(int) * 3);
    void* pos    = stream;

    memcpy(pos, &sc, sizeof(sc_rmem));
    pos += sizeof(sc_rmem);

    memcpy(pos, &arg1, sizeof(int));
    pos += sizeof(int);

    memcpy(pos, &arg2, sizeof(int));
    pos += sizeof(int);

    memcpy(pos, &arg3, sizeof(int));
    pos += sizeof(int);

    return stream;
}

static void deserializar_protocoloMemoria(void* stream, int* arg1, int* arg2, int* arg3) {

    memcpy(arg1 , stream, sizeof(int));
    stream += sizeof(int);
    memcpy(arg2 , stream, sizeof(int));
    stream += sizeof(int);
    memcpy(arg3 , stream, sizeof(int));
   
}


bool send_protocoloMemoria(int fd, sc_rmem co, int arg1, int arg2, int arg3){

    //int a;
    size_t size  = sizeof(sc_rmem) + sizeof(int) * 3;
    void* stream = serializar_protocoloMemoria(co, arg1, arg2, arg3);
//memcpy(&a, stream+(sizeof(sc_rmem) + sizeof(int) * 2), sizeof(int));
    //printf("Valor: %d\n", a);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        printf("Exploto todo\n");
        return false;
    }
    free(stream);
    return true;
}

bool recv_protocoloMemoria(int fd, int* arg1, int* arg2, int* arg3){

    size_t size = sizeof(int) * 3;

    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }

    deserializar_protocoloMemoria(stream, arg1, arg2, arg3);

    free(stream);

    return true;
}



// Protocolo de aceptacion de comunicaciones 
static void* serializar_validacion(sc_val sc) {

    void* stream = malloc(sizeof(op) + sizeof(sc_val));
    void* pos    = stream;

    op cop = VALIDACION;

    memcpy(pos, &cop, sizeof(op));
    pos += sizeof(op);
    
    memcpy(pos, &sc, sizeof(sc_val));
    pos += sizeof(sc_val);


    return stream;
}

static void deserializar_validacion(void* stream, sc_val* sc) {
    op cop;

    memcpy(&cop , stream, sizeof(op));
    stream += sizeof(op);
    memcpy(sc , stream, sizeof(sc_val));


    if(cop != VALIDACION){
        printf("Error de validacion.. recibi cualquier cosa..! (%d)\n", cop);
        *sc = VAL_ERROR;
    }
}


bool send_validacion(int fd){

    size_t size  = sizeof(op) + sizeof(sc_val);
    void* stream = serializar_validacion(VALIDADO);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);

    return true;
}

bool recv_validacion(int fd,sc_val* sc){

    size_t size  = sizeof(op) + sizeof(sc_val);
    void* stream = malloc(size);

    if (recv(fd, stream, size, MSG_WAITALL) != size) {
        free(stream);
        return false;
    }

    deserializar_validacion(stream, sc);

    free(stream);

    return true;
}
