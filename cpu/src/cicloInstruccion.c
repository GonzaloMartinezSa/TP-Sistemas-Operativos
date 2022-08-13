#include "../include/cicloInstruccion.h"
#include <unistd.h>

t_instruccion fetch(t_pcb* pcb);
bool decode(t_instruccion instruccion);
void fetch_operands(t_instruccion* instruccion_copy, int tp_nivel1);
op   execute(t_instruccion instruccion, int tp_nivel1);
void aumentar_program_counter(t_pcb* pcb);
bool check_interrupt();
bool check_exit(op codigo);
bool check_io(op codigo);
void devolver_pcb_a_kernel(t_pcb* pcb, sc_cpu statusCode);
void devolver_pcb_y_tiempo_a_kernel(t_pcb* pcb, sc_cpu statusCode);
int logica_a_fisica(int dirLogica, int tp_nivel1);


// Lo hice asi porque es bastante lineal. Si necesitase, podria cambiarlo a una maquina de estados.
void ciclo_de_instruccion(t_pcb* pcb1){

    //logger_cpu_ciclo = log_create("cpu.log", "CPU - ciclo", 1, LOG_LEVEL_DEBUG);
    op operacion = INSTRUCCION; // default

    sc_cpu statusCode = FINALIZADO; //default


    while(!check_interrupt() && !check_exit(operacion) && !check_io(operacion)){
            
        t_instruccion instruccion = fetch(pcb1);
        bool tieneQueEjecutarseFO = decode(instruccion);
        if(tieneQueEjecutarseFO){
            fetch_operands(&instruccion, pcb1->tp);
        }
        operacion = execute(instruccion, pcb1->tp); // Para chequear la condicion del while

        // Para testear
        log_info(logger_cpu_ciclo, "PROGRAM COUNTER: %d", pcb1->pc);
        //log_info(logger_cpu_ciclo, "Operacion: %d", operacion);
        //log_info(logger_cpu_ciclo, "Hay interrupcion: %d\n", hay_interrupcion);

        if(operacion!=EXIT){
            aumentar_program_counter(pcb1);
        }
    }
    if(check_io(operacion)){
        statusCode = BLOCK;
        devolver_pcb_y_tiempo_a_kernel(pcb1, statusCode);
    }
    else{
        if(check_exit(operacion)){
            statusCode = FINALIZADO;
        }
        else if(check_interrupt()){
            statusCode = IR;
        }
        devolver_pcb_a_kernel(pcb1, statusCode);
    }
}


// TODO

t_instruccion fetch(t_pcb* pcb){
    return (pcb->programa)[pcb->pc];
}

bool decode(t_instruccion instruccion){
    return instruccion.coop==COPY;
}

void fetch_operands(t_instruccion* instruccion_copy, int tp_nivel1){
    //tiene que ir a memoria
    log_warning(logger_cpu_ciclo, "FETCH OPERANDS COPY!");

    int dirFisica = logica_a_fisica(instruccion_copy->arg2, tp_nivel1);
    int valor = accesoAMemUsr(dirFisica, 0, 0, logger_cpu_ciclo);
    instruccion_copy->arg2 = valor; 
}

op   execute(t_instruccion instruccion, int tp_nivel1){

    op codigo_de_operacion = instruccion.coop;

    int dirFisica = 0;
    int valor = 0;
    int rta = 0;

    switch(codigo_de_operacion){
        case NOOP:
            log_warning(logger_cpu_ciclo, "NOOP\n");
            // Agarro el tiempo del config global
            usleep(config_cpu.retardo_noop * 1000);
            break;
        case IO:
            log_warning(logger_cpu_ciclo, "IO\n");
            break;
        case READ:
            // Tiene q acceder a memoria
            log_warning(logger_cpu_ciclo, "READ\n");
            dirFisica = logica_a_fisica(instruccion.arg1, tp_nivel1);
            valor = accesoAMemUsr(dirFisica, 0, 0, logger_cpu_ciclo);
            log_warning(logger_cpu_ciclo, "READ ==> %d", valor);

            break;
        case WRITE:
            // Tiene q acceder a memoria
            log_warning(logger_cpu_ciclo, "WRITE\n");

            dirFisica = logica_a_fisica(instruccion.arg1, tp_nivel1);

            rta = accesoAMemUsr(dirFisica, 1, instruccion.arg2, logger_cpu_ciclo);
            
            if(rta == -1) {
                log_error(logger_cpu_ciclo, "Hubo un error en el Write :(. rta = %d", rta);
            }

            break;
        case COPY:
            // Tiene q acceder a memoria
            printf("\n");
            log_warning(logger_cpu_ciclo, "COPY\n");

            dirFisica = logica_a_fisica(instruccion.arg1, tp_nivel1);
            rta = accesoAMemUsr(dirFisica, 1, instruccion.arg2, logger_cpu_ciclo);
            if(rta == -1) {
                log_error(logger_cpu_ciclo, "Hubo un error en el Copy (la parte de hacer un Write) :(");
            }

            break;
        case EXIT:
            log_warning(logger_cpu_ciclo, "Llego un EXIT gente, se acabo la fiesta\n");
            break; 
    }

    return codigo_de_operacion;
}

void aumentar_program_counter(t_pcb* pcb){
    (pcb->pc)++;
}

bool check_interrupt(){
    // bool global, que es cambiado solo por el thread que escucha la conexion de Interrupt (ademas de aca
    // para resetearla)
    return hay_interrupcion;
}

bool check_exit(op codigo){
    return codigo==EXIT;
}

bool check_io(op codigo){
    return codigo==IO;
}

void devolver_pcb_a_kernel(t_pcb* pcb, sc_cpu statusCode){ 
    // En caso de salir por EXIT o Interrupcion, tiene que devolver al Kernel solo el PCB

    log_warning(logger_cpu_ciclo, "ENVIO PCB!");
    
	uint32_t  tiempoBloqueo = 0;
	send_pcb(conexionKernel_dispatch, pcb, &statusCode, &tiempoBloqueo);
    free(pcb->programa);
}

void devolver_pcb_y_tiempo_a_kernel(t_pcb* pcb, sc_cpu statusCode){
    // En caso de salir por IO, tiene que devolver al Kernel el PCB y Tiempo de Bloqueo
    uint32_t  tiempoBloqueo;
    tiempoBloqueo = (uint32_t)(pcb->programa)[(pcb->pc)-1].arg1;
    log_warning(logger_cpu_ciclo, "ENVIO PCB + TiempoBloqueo: %d ms!", tiempoBloqueo);
    send_pcb(conexionKernel_dispatch, pcb, &statusCode, &tiempoBloqueo);
    free(pcb->programa);
}

int logica_a_fisica(int dirLogica, int tp_nivel1) {

    int dirFisica;
    // Lo busco en TLB. Si esta, actualiza dirFisica. Si no esta, la MMU se encarga de sacar la dirFisica
    if(!buscar_en_tlb(dirLogica, &dirFisica)) {
        log_info(logger_cpu_ciclo, "No hit en la TLB :( ");

        dirFisica = obtenerDirrecionFisica(dirLogica, tp_nivel1, logger_cpu_ciclo);
        cargar_en_tlb(dirLogica / c_memoria.tamano_pagina, (int) dirFisica/1000);

    } else {
        log_info(logger_cpu_ciclo, "HIT TLB!");
    }
    if(dirFisica < 0) {
        log_error(logger_cpu_ciclo, "Direccion Fisica menor a 0");
    }
    /*
    else {
        log_info(logger_cpu_ciclo, "Se obtuvo la direccion fisica y es: %d", dirFisica);
    }
    */
    return dirFisica;
}