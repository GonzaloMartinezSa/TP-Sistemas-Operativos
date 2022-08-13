#include "../include/mmu.h"




//int tp_nivel2 = accesoATabla(tp_nivel1, EntradaPrimerNivel, -1, logger_mmu);

int accesoATabla(int NTP, int ETP, int nivelTabla, t_log* logger_mmu){

    //int socket = crear_conexion(logger_mmu, "memoria", config_cpu.ip_memoria , config_cpu.puerto_memoria);
	
    if(conexionMemoria > 0)
        send_protocoloMemoria(conexionMemoria, ACCESO_A_TABLA, NTP, ETP , nivelTabla);
    else{
        return -1;
    }
	
	sc_rmem cop;
    int arg1, arg2, arg3;

	if (recv(conexionMemoria, &cop, sizeof(sc_rmem), MSG_WAITALL) == sizeof(sc_rmem) && recv_protocoloMemoria(conexionMemoria, &arg1, &arg2, &arg3)) {
        if(cop == SOK){
            //printf("RECIBI arg1:%d arg2:%d arg3:%d", )
            //close(socket); 
            return arg1;
        }
    }
    //close(socket); 
    return -1;
}


int accesoAMemUsr(int dir, int op, int arg, t_log* logger_mmu){
    // op = 0 --> READ
    // op = 1 --> WRITE

    //int socket = crear_conexion(logger_mmu, "memoria", config_cpu.ip_memoria , config_cpu.puerto_memoria);

	if(conexionMemoria > 0)
        send_protocoloMemoria(conexionMemoria, ACCESO_USUARIO, dir, op , arg);
    else{
        return -1;
    }
	
	sc_rmem cop;
    int arg1, arg2, arg3;

	if (recv(conexionMemoria, &cop, sizeof(sc_rmem), MSG_WAITALL) == sizeof(sc_rmem) && recv_protocoloMemoria(conexionMemoria, &arg1, &arg2, &arg3)) {
        if(cop == SOK){
            //close(conexionMemoria);
            return arg1;
        }
    }
    //close(socket);
    return -1;
}

int obtenerDirrecionFisica(int dirLogica, int tp_nivel1, t_log* logger_mmu) {

    int tamPagina = c_memoria.tamano_pagina;
    int cantEntradasTabla = c_memoria.cant_entradas_x_pagina;

    int numPagina = dirLogica / tamPagina;
    int EntradaPrimerNivel = numPagina/cantEntradasTabla;

    int NTP2 = accesoATabla(tp_nivel1, EntradaPrimerNivel, -1, logger_mmu); // Direccion absoluta + relativa
    
    int EntradaSegundoNivel = numPagina%(cantEntradasTabla);

    int marco = accesoATabla(NTP2, EntradaSegundoNivel, tp_nivel1, logger_mmu);

    int desplazamiento = dirLogica - numPagina * tamPagina;

    //log_info(logger_mmu, "\nDir Logica: %d\nNumero de pagina: %d\nDesplazamiento: %d", dirLogica, numPagina, desplazamiento);
    
    return marco * 1000 + desplazamiento; //Dir fisica
}