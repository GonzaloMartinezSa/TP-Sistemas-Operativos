#include "../include/comunicaciones.h"

char eoperacion [][10] = {"READ", "WRITE"};
char copstr [][50]     = {"MEMINFO", "INICIALIZAR PROCESO", "SUSPENDER PROCESO", "FINALIZAR PROCESO", "ACCESO A TABLA", "ACCESO MEM USR"};

int procesar_conexion(int* socket) {

    int cliente_socket = *socket;

    sc_rmem cop = 0;
    int arg1, arg2, arg3, rta;

    while(cliente_socket >= 0) {
        
        if (recv(cliente_socket, &cop, sizeof(sc_rmem), MSG_WAITALL) != sizeof(sc_rmem)) {
            log_info(logger, "DISCONNECT!");
            return 0;
        }

        if(!recv_protocoloMemoria(cliente_socket, &arg1, &arg2, &arg3)){
            log_error(logger, "ERROR EN RECEPCION DE COMANDO!");
            return 0;
        }
        
        log_warning(logger, "SOLICITUD --> %s", copstr[cop]);
        
        sem_wait(&sem);

        switch (cop) {

            case MEMINFO:
                // Clave de validacion
                if(arg1 + arg2 + arg3 == 6){
                    log_warning(logger, "Config CPU!");
                    send_protocoloMemoria(cliente_socket, MEMINFO, conf.tam_pagina, conf.entradas_por_tabla, 123);
                }
                break;
            case INICIALIZAR:
                // PID = arg1
                // Memoria solicitada = arg2
                if(arg1 >= 0 && arg3 == 245){
                    // Chequeo que el proceso no requiera de mas de una TPN1
                    if(arg2 <= MEM_MAX_X_PROCESO){
                        int NTP1 = inicializarProceso(arg2, arg1);
                        log_warning(logger, "Inicializando PID:%d --> %d bytes ==> NTP1:%d", arg1, arg2, NTP1);
                        send_protocoloMemoria(cliente_socket, SOK, NTP1, -1, -1);
                    }
                    else{
                        log_error(logger, "El proceso me solicita mas memoria de la que puedo darle con esta config!");
                        exit(0);
                    }
                    
                }
                break;
            case LIBERAR:
                // PID = arg1
                // Nº TP1 = arg2
                if(arg1 >= 0 && arg2 >= 0 && arg3 == 285){
                    log_warning(logger, "Finalizado PID:%d con NTP1: %d", arg1, arg2);
                    finalizarProceso(arg2);
                    send_protocoloMemoria(cliente_socket, SOK, 285, -1, -1);
                }
                else{
                    log_error(logger, "Error en argumentos de liberacion... %d %d %d", arg1, arg2, arg3);
                    send_protocoloMemoria(cliente_socket, SOK, arg3, -1, -1);
                    exit(0);
                }
                break;
            case SUSPENDERP:
                // PID  = arg1
                // NTP1 = arg2
                if(arg1 >= 0 && arg2 >= 0 && arg3 == 423){
                    log_warning(logger, "Suspendiendo PID:%d con NTP1: %d", arg1, arg2);
                    suspenderProceso(arg2);
                    log_warning(logger, "Suspendido PID:%d con NTP1: %d", arg1, arg2);
                    send_protocoloMemoria(cliente_socket, SOK, arg3, -1, -1);
                }
                break;
            case ACCESO_A_TABLA:
                // NTP  = arg1
                // ETP  = arg2
                // NTP1 = arg3 --> Si es < 0 quiere decir que es un acceso a TP1 
                if(arg1 >= 0 && arg2 >= 0){

                    if(arg3 >= 0)
                        log_info(logger, "Acceso NTP2:%d ETP2:%d", arg1, arg2);
                    else
                        log_info(logger, "Acceso NTP1:%d ETP1:%d", arg1, arg2);
                    
                    
                    int rta = acceso_tabla(arg1, arg2, arg3);

                    send_protocoloMemoria(cliente_socket, SOK, rta , -1, -1);

                    usleep(conf.retardo_por_memoria * 1000); // <-------------------------------------------------------- DELAY ACCESO A MEMORIA
                }
                else{
                    log_error(logger, "ERROR en acceso a tabla!!!");
                    send_protocoloMemoria(cliente_socket, SOK, -1, -1, -1);
                    exit(0);
                }
                break;
            case ACCESO_USUARIO:
                // DireccionFisica  = arg1
                // Operacion        = arg2 --> 0 = READ ; 1 = WRITE
                // Argumento        = arg3 --> Me sirve solo para WRITE
                if(arg1 >= 0 && arg2 <= 2){
                    //log_info(logger, "MEM-USR %s (DIR FISICA:%d) --> ARG %d",eoperacion[arg2],arg1 ,arg3);
                
                    // Desarmo la dir fisica en marco y desplazamiento
                    int marco = (int) (arg1 / 1000);
                    int desplazamiento = arg1%1000; 

                    if(arg2 == 0){
                        uint32_t rta;
                        acceso_memoria_usr(OPM_READ, marco, desplazamiento, &rta);
                        log_warning(logger, "READ de direccion fisica %d --> RTA = %d\n\n", arg1, (int)rta);
                        send_protocoloMemoria(cliente_socket, SOK, (int)rta, -1, -1);
                    }
                    else{
                        uint32_t insert = (uint32_t)arg3;
                        int rta = acceso_memoria_usr(OPM_WRITE, marco, desplazamiento, &insert);
                        log_warning(logger, "WRITE de direccion fisica %d (Marco %d Desplazamiento %d) con valor %d\n\n", arg1, marco, desplazamiento, insert);
                        send_protocoloMemoria(cliente_socket, SOK, (int)arg2, -1, -1);
                    }
                    usleep(conf.retardo_por_memoria * 1000); // <------------------------------------------------------------------------- DELAY MEMORIA USR
                }
                else
                    send_protocoloMemoria(cliente_socket, SOK, -1, -1, -1);  
                break; 
            default:
                log_error(logger, "ERROR EN CODIGO DE OPERACION");
        }

        sem_post(&sem);
    }

    return 1;
}
